#include "../inc/nes_ppu.hpp"

nes_ppu::nes_ppu(){
    registers = {};
    vram.resize(VRAM_SIZE);
    chrrom.resize(CHROM_SIZE);
    palette_table.resize(PALETTE_SIZE);
    oam_data.resize(OAM_SIZE);
    ppu_cycle = pixel = scanline = 0;
}

nes_ppu::~nes_ppu(){

}

/*
    Connects PPU to other components and to system
*/
void nes_ppu::turn_on(nes_system *sys){
    _nes_system = sys;
    cpu = _nes_system->cpu();
    frame = _nes_system->get_Frame();
}

void nes_ppu::reset(){

}

void nes_ppu::step_to(uint64_t master_cycle){
    while(ppu_cycle <= master_cycle*3){
        process_pixel();
    }
}

void nes_ppu::process_pixel(){
    pixel+=1;
    if(pixel >= 341){
        pixel = 0;
        scanline += 1;

        if(scanline == 241){
            if(registers.PPUCTRL & 0x80){
                registers.PPUSTATUS |= 0x80;
                cpu->set_NMI();
            }
        }

        if(scanline == 262){

        }
    }
}

/*
    Loads CHR-ROM
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
    Setter/Getter for regular ppu registers
*/
void nes_ppu::PPUCTRL_w(uint8_t val){
    if(~registers.PPUCTRL & val & CTRL_VBLANK) cpu->set_NMI();
    registers.PPUCTRL = val;
}

void nes_ppu::PPUMASK_w(uint8_t val){ registers.PPUMASK = val; }

uint8_t nes_ppu::PPUSTATUS_r(){
    reset_latch(); 
    return registers.PPUSTATUS;
}

uint8_t& nes_ppu::OAMADDR(){ return registers.OAMADDR; }
uint8_t& nes_ppu::OAMDATA(){ return registers.OAMDATA; }

void nes_ppu::OAMDMA_w(uint8_t val){ 
    registers.OAMDMA = val;
    //cpu->set_dma_req(val);
    cpu->set_dma_req();
}

/*
    Write into VRAM through memory-mapped registers
*/
void nes_ppu::write_data(uint8_t val){
    uint16_t addr = get_PPUADDR();
    inc_PPUADDR();

    if(addr < 0x2000){
        cout<<"Writing to non-modifiable CHR-ROM or Palette Table..."<<endl;
        exit(1);
    }else if(addr < 0x3F00){
        vram[mirror_addr(addr)] = val;
    }else{
        palette_table[(addr-0x3F00) % 0x20];
    }
    
}

/*

*/
uint8_t nes_ppu::read_data(){

    uint16_t addr = get_PPUADDR();
    uint8_t result = 0;
    inc_PPUADDR();

    if(addr < 0x2000){
        result = read_buffer;
        read_buffer = chrrom[addr];
        return result;
    }else if(addr < 0x3F00){
        result = read_buffer;
        read_buffer = vram[mirror_addr(addr)];
        return result;
    }else{
        return palette_table[(addr-0x3F00) % 0x20];
    }
}

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

        default:
            cout<<"Error in address..."<<endl;
            exit(1);
    }
    return vram_addr;
}

/*
    Set both bytes of PPUADDR at once
*/
void nes_ppu::set_PPUADDR(uint16_t val){
    registers.PPUADDR.value[0] = (val & 0xFF00)>>8;
    registers.PPUADDR.value[1] = val & 0x00FF;
}

/*
    Return PPUADDR as a single number
*/
uint16_t nes_ppu::get_PPUADDR(){
    return (registers.PPUADDR.value[0]<<8) | registers.PPUADDR.value[1]; 
}

/*
    Update one of two bytes of PPUADDR, mirrored to be inside 0x0000-0x3FFF
*/
void nes_ppu::PPUADDR_w(uint8_t val){
    if(registers.PPUADDR.hi)
        registers.PPUADDR.value[0] = val;
    else
        registers.PPUADDR.value[1] = val;
    
    // Mirror address
    if(get_PPUADDR() > 0x3FFF) 
        set_PPUADDR(get_PPUADDR()&0x3FFF);

    registers.PPUADDR.hi = !registers.PPUADDR.hi;
}

uint8_t nes_ppu::vram_increment(){
    if((registers.PPUCTRL & 0x04) == 0x04) return 32;
    else return 1;
}

/*
    Increment PPUADDR by 1 or 32
*/
void nes_ppu::inc_PPUADDR(){
    uint8_t lo_byte = registers.PPUADDR.value[1];
    registers.PPUADDR.value[1] += vram_increment();

    if(lo_byte >= registers.PPUADDR.value[1]){
        registers.PPUADDR.value[0] += 1;
        registers.PPUADDR.value[0] %= 0x40;
    }
}

/*
    Resets condition to retrieve first (hi) byte for PPUADDR
*/
void nes_ppu::reset_latch(){
    registers.PPUADDR.hi = true;
}