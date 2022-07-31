#ifndef NES_PPU_HPP_
#define NES_PPU_HPP_

#include "nes_system.hpp"
#include "nes_cpu.hpp"
#include "nes_component.hpp"
#include "nes_mapper.hpp"
#include "frame.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>
#include <tuple>

#define CHROM_SIZE      0x2000
#define VRAM_SIZE       0x800
#define PALETTE_SIZE    0x20
#define OAM_SIZE        0x100

#define CTRL_VBLANK     0x80

using namespace std;

class nes_system;
class nes_cpu;

struct large_reg{
    uint8_t value[2] = {};
    bool hi = true;
};

struct PPU_REGS{
    uint8_t PPUCTRL;            // > write       $2000 Control Register       VPHB SINN
    uint8_t PPUMASK;            // > write       $2001 Mask Register          BGRs bMmG
    uint8_t PPUSTATUS;          // < read        $2002 Status Register        VSO. ....
    uint8_t OAMADDR;            // > write       $2003 OAM Address Register
    uint8_t OAMDATA;            // <> read/write $2004 OAM Data Port
    large_reg PPUSCROLL;        // >> write x2   $2005 Scrolling Register
    large_reg PPUADDR;          // >> write x2   $2006 VRAM Address Register
    uint8_t PPUDATA;            // <> read/write $2007 PPU Data Port
    uint8_t OAMDMA;             // > write       $4014 OAM DMA Register (high byte of CPU address: Writing $XX will write contents of CPU page $XX00 - $XXFF (256 B)
};

class nes_ppu: public nes_component{
private:
    nes_system *_nes_system;
    nes_cpu *cpu;

    PPU_REGS registers;
    uint8_t read_buffer;
    
    // memory
    
    vector<uint8_t> palette_table;
    vector<uint8_t> vram;
    vector<uint8_t> oam_data;

    //cycle info
    uint64_t ppu_cycle;

    //current pixel being processed
    uint16_t pixel;
    uint16_t scanline;

    // true = vertical mirror, false = horizontal mirror
    bool vert_mirror; 

public:
    // holds data to be rendered
    vector<uint8_t> chrrom;
    Frame* frame;

private:
    uint8_t vram_increment();
    uint8_t increment_addr();
    void set_PPUADDR(uint16_t val);
    void inc_PPUADDR();
    uint16_t get_PPUADDR();
    void reset_latch();

    uint16_t mirror_addr(uint16_t addr);

public:
    nes_ppu();
    ~nes_ppu();
    virtual void turn_on(nes_system *sys);
    virtual void reset();
    virtual void step_to(uint64_t master_cycle);
    void process_pixel();

    void load_chrrom(vector<uint8_t> _chrrom);

    void set_mirroring(bool _mirror);

    void PPUCTRL_w(uint8_t val);        
    void PPUMASK_w(uint8_t val);     
    uint8_t PPUSTATUS_r();
    uint8_t& OAMADDR();
    uint8_t& OAMDATA();
    void OAMDMA_w(uint8_t val);
    uint8_t read_data();
    void write_data(uint8_t val);

    void PPUADDR_w(uint8_t val);

};

#endif