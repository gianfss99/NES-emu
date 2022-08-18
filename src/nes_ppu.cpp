#include "../inc/nes_ppu.hpp"

nes_ppu::nes_ppu()
{
    registers = {};
    vram.resize(VRAM_SIZE);
    chrrom.resize(CHROM_SIZE);
    palette_table.resize(PALETTE_SIZE);
    oam_data.resize(OAM_SIZE);
    ppu_cycle = dot = 0;
    scanline = 0;
}

nes_ppu::~nes_ppu()
{

}

/*
    Connects PPU to other components and to system
*/
void nes_ppu::turn_on(nes_system *sys)
{
    _nes_system = sys;
    cpu = _nes_system->cpu();
    frame = _nes_system->get_Frame();
}

void nes_ppu::reset()
{

}

void nes_ppu::step_to(uint64_t master_cycle)
{
    while(ppu_cycle <= master_cycle*3)
    {
        process_pixel();
    }
}

uint8_t nes_ppu::cpu_read(uint16_t addr)
{
    uint8_t data = 0x0;
    switch (addr)
    {
        case 0x00: break;
        case 0x01: break;

        case 0x02:
            data = (registers.status & 0xE0) | (data_buffer & 0x1F);
            address_latch = 0;
            registers.status &= ~(0x80);
            //std::cout<<"STATUS read"<<std::endl;
            break;
        case 0x03:
            break;
        case 0x04:
            data = oam_data[oam_addr];
            // if((registers.status & 0x80) == 0) 
            //     oam_addr++;
            break;
        case 0x05:
            break;
        case 0x06:
            break;
        case 0x07:
            data = data_buffer;
            data_buffer = ppu_read(vram_addr.reg);
            if(vram_addr.reg >= 0x3F00) {
                data = read_palette(vram_addr.reg);
                //std::cout<<"Hello"<<std::endl;
                //data_buffer = data;
            }
            
            vram_addr.reg += vram_increment();
            break;
    }

    return data;
}

void nes_ppu::cpu_write(uint16_t addr, uint8_t data){
    switch(addr)
    {
        case 0x00: //Control
            registers.control = data;
            tram_addr.nametable_x = registers.control & 0x1;
            tram_addr.nametable_y = (registers.control & 0x2)>>1;
            break;
        
        case 0x01: // Mask
            registers.mask = data;
            break;
        
        case 0x02: // Status
            break;
        
        case 0x03: // OAM Address
            oam_addr = data;
            break;
        
        case 0x04: // OAM Data
            oam_data[oam_addr] = data;
            oam_addr++;
            break;
        
        case 0x05: // Scroll
            if(!address_latch)
            {
                fine_x = data & 0x07;
                tram_addr.coarse_x = data >> 3;
                address_latch = 1;
            }else
            {
                tram_addr.fine_y = data & 0x07;
                tram_addr.coarse_y = data >> 3;
                address_latch = 0;
            }
            break;
        
        case 0x06:
            if(!address_latch)
            {
                tram_addr.reg = (uint16_t)((data & 0x3F)<<8) | (tram_addr.reg & 0xFF);
                address_latch = 1;
            }else
            {
                tram_addr.reg = (tram_addr.reg & 0xFF00) | data;
                vram_addr = tram_addr;
                address_latch = 0;
            }
            break;
        
        case 0x07:
                ppu_write(vram_addr.reg, data);
                vram_addr.reg += vram_increment();
                break;
    }
}

/*

*/
void nes_ppu::ppu_write(uint16_t addr, uint8_t data){

    addr &= 0x3FFF;

    if(addr < 0x2000)
    {
        chrrom[addr] = data;
    }

    else if(addr < 0x3F00)
    {
        vram[mirror_addr(addr)] = data;
    }

    else
    {
        addr &= 0x1F;
        if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;
        palette_table[addr] = data;
    }
}

uint8_t nes_ppu::ppu_read(uint16_t addr)
{

    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if(addr < 0x2000)
    {
        data = chrrom[addr];
        
    }
    
    else if(addr <= 0x3FFF)
    {
        data = vram[mirror_addr(addr)]; 
    }
    
    return data;

}

/*
    read 
*/
uint8_t nes_ppu::read_palette(uint16_t addr){

    uint8_t data = 0x0;
    addr &= 0x1F;

    if (addr == 0x0010) addr = 0x0000;
    if (addr == 0x0014) addr = 0x0004;
    if (addr == 0x0018) addr = 0x0008;
    if (addr == 0x001C) addr = 0x000C;

    data = palette_table[addr];
    return data;
}

void nes_ppu::process_pixel()
{

    auto TracePPU = [&]()
    {
        // traces PPU registers for debugging
        std::cout<<std::hex<<"Ctrl:"<<(uint16_t)registers.control<<" Mask:"<<(uint16_t)registers.mask<<" Status:"<<(uint16_t)registers.status<<std::endl;
        std::cout<<std::hex<<"vram_addr:"<<vram_addr.reg<<" tram_addr:"<<tram_addr.reg<<std::endl;
        if(shift_reg.hi_tile > 0 || shift_reg.lo_tile>0)
            std::cout<<std::hex<<"bg_lo_sh:"<<shift_reg.lo_tile<<" bg_hi_sh:"<<shift_reg.hi_tile<<std::endl;
        if(shift_reg.hi_attr > 0 || shift_reg.lo_attr>0)
            std::cout<<std::hex<<"attr_lo_sh:"<<shift_reg.lo_attr<<" attr_hi_sh:"<<shift_reg.hi_attr<<std::endl;
        std::cout<<std::dec<<"cycle: "<<ppu_cycle<<" dot:"<<dot<<" scanline:"<<scanline<<std::endl;
        std::cout<<"_________________________________________"<<std::endl;
    };
    
    auto IncrementScrollX = [&]()
    {
        // increase coarse x until it overflows and change nametable for horizontal scrolling
        if (((registers.mask & RENDER_BG) > 0) || ((registers.mask & RENDER_SPRITE) > 0))
        {
            if (vram_addr.coarse_x == 31)
            {
                vram_addr.coarse_x == 0;
                vram_addr.nametable_x = ~vram_addr.nametable_x;
            }
            else
            {
                vram_addr.coarse_x++;
            }
        }
    };

    auto IncrementScrollY = [&]()
    {
        // increases fine_y until it overflows, then increase course_y until it overflows and reset
        // change nametable for vertical scrolling
        if (((registers.mask & RENDER_BG) > 0) || ((registers.mask & RENDER_SPRITE) > 0))
        {
            if(vram_addr.fine_y < 7)
            {
                vram_addr.fine_y++;

            }
            else
            {
                vram_addr.fine_y = 0;
                if(vram_addr.coarse_y == 29)
                {
                    vram_addr.coarse_y = 0;
                    vram_addr.nametable_y = ~vram_addr.nametable_y;
                }
                else if(vram_addr.coarse_y == 31)
                {
                    vram_addr.coarse_y = 0;
                }
                else
                {
                    vram_addr.coarse_y++;
                }
            }
        }
    };

    auto TransferAddrX = [&]()
    {
        // end of scanline, resets VRAM address register to the leftmost part of the screen
        if (((registers.mask & RENDER_BG) > 0) || ((registers.mask & RENDER_SPRITE) > 0))
        {
            vram_addr.nametable_x = tram_addr.nametable_x;
			vram_addr.coarse_x    = tram_addr.coarse_x;
        }
    };

    auto TransferAddrY = [&]()
    {
        // end of frame, resets VRAM address register to the top of the screen
        if (((registers.mask & RENDER_BG) > 0) || ((registers.mask & RENDER_SPRITE) > 0))
        {
            vram_addr.nametable_y = tram_addr.nametable_y;
			vram_addr.coarse_y    = tram_addr.coarse_y;
            vram_addr.fine_y = tram_addr.fine_y;
        }
    };

    auto LoadNextTile = [&]()
    {
        // loads next 8 pixels to shift registers
        shift_reg.lo_tile = (shift_reg.lo_tile & 0xFF00) | lo_bg_byte;
        shift_reg.hi_tile = (shift_reg.hi_tile & 0xFF00) | hi_bg_byte;

        // every tile has the same palette
        // loads 1 to all 8 bits if mask is true, else set to 0
        shift_reg.lo_attr = (shift_reg.lo_attr & 0xFF00) | ((attr_byte & 0x1) ? 0xFF : 0x0);
        shift_reg.hi_attr = (shift_reg.hi_attr & 0xFF00) | ((attr_byte & 0x2) ? 0xFF : 0x0);
    };

    auto UpdateShiftRegs = [&]()
	{
        // sets next pixel to render from shift registers
		if ((registers.mask & RENDER_BG) > 0)
		{
			// Shifting background tile pattern row
			shift_reg.lo_tile <<= 1;
			shift_reg.hi_tile <<= 1;

			// Shifting palette attributes by 1
			shift_reg.lo_attr <<= 1;
			shift_reg.hi_attr <<= 1;
		}

        if(((registers.mask & RENDER_SPRITE) > 0) && dot >= 1 && dot < 258){
            // shift 
            for(int i = 0; i<sprite_count; i++)
            {
                if(secondary_oam[4*i+3] > 0){
                    secondary_oam[4*i+3]--;
                }
                else
                {
                    sprite_shifter_hi[i] <<= 1;
                    sprite_shifter_lo[i] <<= 1;
                }
            }
        }
	};    
    

    if(scanline >= -1 && scanline < 240)
    {
        if(scanline == 0 && dot == 0)
        {
            dot = 1;
        }

        if(scanline == -1 && dot == 1)
        {
            registers.status &= ~(0x80);
            registers.status &= ~(0x20);

            for(int i = 0; i<8; i++){
                sprite_shifter_hi[i] = 0;
                sprite_shifter_lo[i] = 0;
            }
        }

        // rendering cycles
        if((dot >= 2 && dot < 258) || (dot >= 321 && dot < 338))
        {
            UpdateShiftRegs();
            switch((dot - 1) % 8)
            {
                case 0:
                    //load next tile to shifter
                    LoadNextTile();
                    // store pattern table index from VRAM
                    nt_byte = ppu_read(0x2000 | (vram_addr.reg & 0xFFF));
                    break;
                case 2:
                    // store attribute table (color) byte from VRAM
                    attr_byte = ppu_read(0x23C0 | (vram_addr.nametable_y << 11)
                                                    | (vram_addr.nametable_x << 10)
                                                    | ((vram_addr.coarse_y >> 2) << 3)
                                                    | (vram_addr.coarse_x >> 2));
                    
                    if (vram_addr.coarse_y & 0x2) attr_byte >>= 4;
                    if (vram_addr.coarse_x & 0x2) attr_byte >>= 2;
                    attr_byte &= 0x3;
                    
                    break;
                case 4:
                    // store lower byte from pattern table
                    lo_bg_byte = ppu_read(((registers.control & 0x10)<<8)
                                            + ((uint16_t)nt_byte << 4)
                                            + (vram_addr.fine_y) + 0);
                    break;
                case 6:
                    //store higher byte from pattern table
                    hi_bg_byte = ppu_read(((registers.control & 0x10)<<8)
                                            + ((uint16_t)nt_byte << 4)
                                            + (vram_addr.fine_y) + 8);
                    break;
                case 7:
                    // go to next tile (increasing coarse x)
                    IncrementScrollX();
                    break;
            }
        }

        if(dot == 256)
        {
            // go to next scanline/tile (increase fine y and coarse y)
            IncrementScrollY();
        }

        if(dot == 257)
        {
            // load tile bytes to shift registers
            //LoadNextTile();
            // return coarse x to the leftmost part of the screen
            TransferAddrX();
        }

        if(dot >= 257 && dot < 321)
        {
            oam_addr = 0;
        }

        if(dot == 338 || dot == 340)
        {
            nt_byte = ppu_read(0x2000+(vram_addr.reg & 0xFFF));
        }

        if(scanline == -1 && dot >= 280 && dot < 305)
        {
            //return coarse y to the uppermost part of the screen
            TransferAddrY();
        }

        if(scanline >= 0 && dot == 257){

            std::memset(secondary_oam,0xFF,32 * sizeof(uint8_t));
            sprite_count = 0;

            uint8_t n = 0;
            while(n < 64 && sprite_count <= 8)
            {
                int16_t diff = ((int16_t)scanline - (int16_t)oam_data[4*n]);

                if(diff >=0 && diff <=7)
                {
                    for(int i = 0; i<4; i++)
                    {
                        if(4*sprite_count > 31) break;
                        secondary_oam[4*sprite_count + i] = oam_data[4*n + i];
                    }
                    sprite_count++;
                }
                n++;
            }

            if (sprite_count>8)
            {
                registers.status |= 0x20;
                sprite_count = 8;
            }
            else
            {
                registers.status &= ~(0x20);
            }
        }

        if (dot == 340){
            for (int i = 0; i<sprite_count; i++){
                uint8_t sprite_hi_byte, sprite_lo_byte;
                uint16_t sprite_hi_addr, sprite_lo_addr;

                // 8x8 tile mode
                if(!(registers.control & 0x20))
                {
                    if(!(secondary_oam[4*i+2] & 0x80))
                    {
                        //sprite not flipped vertically
                        sprite_lo_addr = ((registers.control & 0x08)<<8
                                        | ((uint16_t)secondary_oam[4*i + 1] << 4)
                                        | (scanline - secondary_oam[4*i]));
                    }
                    else{
                        // flipped vertically
                        sprite_lo_addr = ((registers.control & 0x08)<<8
                                        | ((uint16_t)secondary_oam[4*i + 1] << 4)
                                        | (7-(scanline - secondary_oam[4*i])));
                    }
                }
                //8x16 tile mode
                else
                {
                    //TODO
                }

                sprite_hi_addr = sprite_lo_addr + 8;
                sprite_hi_byte = ppu_read(sprite_hi_addr);
                sprite_lo_byte = ppu_read(sprite_lo_addr);

                auto reverse_byte = [](uint8_t b){
                    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
                    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
                    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
                    return b;
                };

                // flip horizontally
                if(secondary_oam[4*i + 2] & 0x40){
                    sprite_hi_byte = reverse_byte(sprite_hi_byte);
                    sprite_lo_byte = reverse_byte(sprite_lo_byte);

                }

                sprite_shifter_hi[i] = sprite_hi_byte;
                sprite_shifter_lo[i] = sprite_lo_byte;

            }
        }
    }

    if(scanline == 240){
        //do nothing
    }

    // VBlank scanlines
    if((scanline >= 241) && (scanline < 261))
    {
        if((scanline == 241) && (dot == 1))
        {
            registers.status |= 0x80; // set VBlank flag
            
            if((registers.control & 0x80) > 0)
            {
                cpu->set_NMI(); // request NMI (Non-Maskable Interrupt) to CPU
            }
        }
    }
    
    uint8_t bg_pixel = 0x0;
    uint8_t bg_palette = 0x0;

    if((registers.mask & RENDER_BG))
    {
        // Scrolling mux
        uint16_t mux = 0x8000 >> fine_x;

        // Get the pixel palette value and what palette table it comes from
        bg_pixel = (((shift_reg.hi_tile & mux) > 0) << 1) | ((shift_reg.lo_tile & mux) > 0);
        bg_palette = (((shift_reg.hi_attr & mux) > 0) << 1) | ((shift_reg.lo_attr & mux) > 0);

    }
    

    uint8_t sprite_pixel = 0x0;
    uint8_t sprite_palette = 0x0;
    uint8_t sprite_priority = 0x0;

    if((registers.mask & RENDER_SPRITE) > 0)
    {
        // Get the first sprite that should be rendered at this cycle
        // only the one with the highest priority and that is not transparent is shown
        for(int i = 0; i<sprite_count; i++){
            if(secondary_oam[4*i + 3] == 0){
                sprite_pixel = (((sprite_shifter_hi[i] & 0x80) > 0) << 1) | ((sprite_shifter_lo[i] & 0x80) > 0);
                sprite_palette = (secondary_oam[4*i + 2] & 0x3) + 0x04;
                sprite_priority = (secondary_oam[4*i + 2] & 0x20) == 0;

                if(sprite_pixel != 0) break;
                
            }
        }
    }

    uint8_t pixel = 0;
    uint8_t palette = 0;

    // select either the background or sprite pixel to render
    // a pixel == 0 means it is transparent/rendering is disabled
    if(bg_pixel == 0 && sprite_pixel == 0)
    {
        pixel = 0;
        palette = 0;
    }
    else if(bg_pixel == 0 && sprite_pixel > 0)
    {
        pixel = sprite_pixel;
        palette = sprite_palette;
    }
    else if(bg_pixel > 0 && sprite_pixel == 0)
    {
        pixel = bg_pixel;
        palette = bg_palette;
    }
    else if(bg_pixel > 0 && sprite_pixel > 0){
        if(sprite_priority)
        {
            pixel = sprite_pixel;
            palette = sprite_palette;
        }
        else
        {
            pixel = bg_pixel;
            palette = bg_palette;
        }
    }

    // get the color for the current pixel
    uint16_t palette_index = read_palette(0x3F00 + (palette << 2) + pixel) & 0x3F;
    tuple<uint8_t,uint8_t,uint8_t> color = SYSTEM_PALETTE[palette_index];
    
    //TracePPU();
    frame->set_pixel(dot-1,scanline,color);

    dot++;
    if (dot >= 341){
        dot = 0;
        scanline++;
        
        if(scanline >= 261){
            scanline = -1;
            _nes_system->frame_completed();
        }
    }
    ppu_cycle++;
}

/*
    Loads Character ROM
*/
void nes_ppu::load_chrrom(vector<uint8_t> _chrrom){
    memcpy(&chrrom[0],_chrrom.data(),_chrrom.size());
}

/*
    Set Vertical(1) or Horizontal(0) mirroring for nametables
*/
void nes_ppu::set_mirroring(bool _mirror){
    vert_mirror = _mirror;
}

/*

*/
void nes_ppu::OAMDMA_w(uint8_t val){ 
    registers.OAMDMA = val;
    cpu->set_dma_req();
}

/*
    Mirrors the address inside VRAm depending on the mirroring
    set in the .nes file header
*/
uint16_t nes_ppu::mirror_addr(uint16_t addr){
    uint16_t mirror_down = addr & 0x2FFF; //
    uint16_t vram_addr = mirror_down - 0x2000;
    uint8_t nametable = vram_addr / 0x400;
    switch(nametable){
        case 1:
            if(!vert_mirror) vram_addr -= 0x400;
            break;
        
        case 2:
            if(vert_mirror) vram_addr -= 0x800;
            else vram_addr -= 0x400;
            break;

        case 3:
            vram_addr -= 0x800;
            break;
    }
    return vram_addr;
}

/*
    Jump VRAM to next scanline (32) or next pixel (1)
*/
uint8_t nes_ppu::vram_increment(){
    return ((registers.control & 0x04) == 0x04) ? 32 : 1;
}