#include "../inc/nes_ppu.hpp"

nes_ppu::nes_ppu(){
    registers = {};
    vram.resize(0x800);
    chrrom.resize(0x2000);
    palette_table.resize(0x20);
    oam_data.resize(0x100);
}

nes_ppu::~nes_ppu(){

}

/*
    Connects PPU to other components and to system
*/
void nes_ppu::turn_on(nes_system *sys){
    _nes_system = sys;
    cpu = _nes_system->cpu();
}

void nes_ppu::reset(){

}

void nes_ppu::step_to(int _cycle){

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
uint8_t& nes_ppu::PPUCTRL(){ return registers.PPUCTRL; }
uint8_t& nes_ppu::PPUMASK(){ return registers.PPUMASK; }
uint8_t& nes_ppu::PPUSTATUS(){ return registers.PPUSTATUS; }
uint8_t& nes_ppu::OAMADDR(){ return registers.OAMADDR; }
uint8_t& nes_ppu::OAMDATA(){ return registers.OAMDATA; }
uint8_t& nes_ppu::PPUSCROLL(){ return registers.PPUSCROLL; }
uint8_t& nes_ppu::PPUDATA(){ return registers.PPUDATA; }
uint8_t& nes_ppu::OAMDMA(){ return registers.OAMDMA; }

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
    return registers.PPUADDR.value[0]<<8 | registers.PPUADDR.value[1]; 
}

/*
    Update one of two bytes of PPUADDR, mirrored to be inside 0x0000-0x3FFF
*/
void nes_ppu::update_PPUADDR(uint8_t val){
    if(registers.PPUADDR.hi)
        registers.PPUADDR.value[0] = val;
    else
        registers.PPUADDR.value[1] = val;
    
    // Mirror address
    if(get_PPUADDR() > 0x3FFF) 
        set_PPUADDR(get_PPUADDR()&0x3FFF);

    registers.PPUADDR.hi = !registers.PPUADDR.hi;
}


/*
    Increment PPUADDR by 1 or 32
*/
void nes_ppu::inc_PPUADDR(){
    uint8_t lo_byte = registers.PPUADDR.value[1];
    registers.PPUADDR.value[1] += vram_increment();

    if(lo_byte >= registers.PPUADDR.value[1])
        registers.PPUADDR.value[0] += 1;
    
}

/*
    Resets condition to retrieve first (hi) byte for PPUADDR
*/
void nes_ppu::reset_PPUADDR(){
    registers.PPUADDR.hi = true;
}