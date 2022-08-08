#ifndef NES_PPU_HPP_
#define NES_PPU_HPP_

#include "nes_system.hpp"
#include "nes_cpu.hpp"
#include "nes_component.hpp"
#include "nes_mapper.hpp"
#include "frame.hpp"
#include "helper.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>
#include <tuple>



#define CHROM_SIZE      0x2000
#define VRAM_SIZE       0x800
#define PALETTE_SIZE    0x20
#define OAM_SIZE        0x100

#define CTRL_VBLANK     0x80
#define COARSE_X        0x1F
#define COARSE_Y        0x3E0
#define NT_X            0x400
#define NT_Y            0x800

#define RENDER_BG       0x08
#define RENDER_SPRITE   0x10

using namespace std;

class nes_system;
class nes_cpu;

struct large_reg{
    uint8_t value[2] = {};
    bool hi = true;
};



struct PPU_REGS{
    uint8_t control;            // > write       $2000 Control Register       VPHB SINN
    uint8_t mask;            // > write       $2001 Mask Register          BGRs bMmG
    uint8_t status;          // < read        $2002 Status Register        VSO. ....
    uint8_t oam_addr;            // > write       $2003 OAM Address Register
    uint8_t oam_data;            // <> read/write $2004 OAM Data Port
    uint8_t OAMDMA;             // > write       $4014 OAM DMA Register (high byte of CPU address: Writing $XX will write contents of CPU page $XX00 - $XXFF (256 B)
};



class nes_ppu: public nes_component{
private:
    vector<tuple<uint8_t,uint8_t,uint8_t>> SYSTEM_PALETTE = {
        make_tuple(0x80, 0x80, 0x80), make_tuple(0x00, 0x3D, 0xA6), make_tuple(0x00, 0x12, 0xB0), make_tuple(0x44, 0x00, 0x96), make_tuple(0xA1, 0x00, 0x5E),
        make_tuple(0xC7, 0x00, 0x28), make_tuple(0xBA, 0x06, 0x00), make_tuple(0x8C, 0x17, 0x00), make_tuple(0x5C, 0x2F, 0x00), make_tuple(0x10, 0x45, 0x00),
        make_tuple(0x05, 0x4A, 0x00), make_tuple(0x00, 0x47, 0x2E), make_tuple(0x00, 0x41, 0x66), make_tuple(0x00, 0x00, 0x00), make_tuple(0x05, 0x05, 0x05),
        make_tuple(0x05, 0x05, 0x05), make_tuple(0xC7, 0xC7, 0xC7), make_tuple(0x00, 0x77, 0xFF), make_tuple(0x21, 0x55, 0xFF), make_tuple(0x82, 0x37, 0xFA),
        make_tuple(0xEB, 0x2F, 0xB5), make_tuple(0xFF, 0x29, 0x50), make_tuple(0xFF, 0x22, 0x00), make_tuple(0xD6, 0x32, 0x00), make_tuple(0xC4, 0x62, 0x00),
        make_tuple(0x35, 0x80, 0x00), make_tuple(0x05, 0x8F, 0x00), make_tuple(0x00, 0x8A, 0x55), make_tuple(0x00, 0x99, 0xCC), make_tuple(0x21, 0x21, 0x21),
        make_tuple(0x09, 0x09, 0x09), make_tuple(0x09, 0x09, 0x09), make_tuple(0xFF, 0xFF, 0xFF), make_tuple(0x0F, 0xD7, 0xFF), make_tuple(0x69, 0xA2, 0xFF),
        make_tuple(0xD4, 0x80, 0xFF), make_tuple(0xFF, 0x45, 0xF3), make_tuple(0xFF, 0x61, 0x8B), make_tuple(0xFF, 0x88, 0x33), make_tuple(0xFF, 0x9C, 0x12),
        make_tuple(0xFA, 0xBC, 0x20), make_tuple(0x9F, 0xE3, 0x0E), make_tuple(0x2B, 0xF0, 0x35), make_tuple(0x0C, 0xF0, 0xA4), make_tuple(0x05, 0xFB, 0xFF),
        make_tuple(0x5E, 0x5E, 0x5E), make_tuple(0x0D, 0x0D, 0x0D), make_tuple(0x0D, 0x0D, 0x0D), make_tuple(0xFF, 0xFF, 0xFF), make_tuple(0xA6, 0xFC, 0xFF),
        make_tuple(0xB3, 0xEC, 0xFF), make_tuple(0xDA, 0xAB, 0xEB), make_tuple(0xFF, 0xA8, 0xF9), make_tuple(0xFF, 0xAB, 0xB3), make_tuple(0xFF, 0xD2, 0xB0),
        make_tuple(0xFF, 0xEF, 0xA6), make_tuple(0xFF, 0xF7, 0x9C), make_tuple(0xD7, 0xE8, 0x95), make_tuple(0xA6, 0xED, 0xAF), make_tuple(0xA2, 0xF2, 0xDA),
        make_tuple(0x99, 0xFF, 0xFC), make_tuple(0xDD, 0xDD, 0xDD), make_tuple(0x11, 0x11, 0x11), make_tuple(0x11, 0x11, 0x11),
    };
    nes_system *_nes_system;
    nes_cpu *cpu;

    PPU_REGS registers;
    uint8_t data_buffer;
    struct shift_registers{
        uint16_t lo_attr = 0x00;
        uint16_t hi_attr = 0x00;
        uint16_t lo_tile = 0x00;
        uint16_t hi_tile = 0x00;
    } shift_reg;
    

    //thanks to loopy for this implementation of PPUADDR
    union loopy_register{
        struct{
            uint16_t coarse_x : 5;
            uint16_t coarse_y : 5;
            uint16_t nametable_x : 1;
            uint16_t nametable_y : 1;
            uint16_t fine_y : 3;
            uint16_t unused : 1;
        };
        uint16_t reg = 0x0000;
    };

    loopy_register vram_addr; //active register
    loopy_register tram_addr; //temporary register

    bool address_latch = 0x0;
    uint8_t fine_x = 0x00;

    uint8_t oam_addr = 0x00;
    
    // memory
    
    vector<uint8_t> palette_table;
    vector<uint8_t> vram;
    vector<uint8_t> oam_data;

    //cycle info
    uint64_t ppu_cycle;

    //current pixel being processed
    uint16_t dot;
    signed short scanline;
    // uint8_t tile_x;
    // uint8_t tile_y;
    // uint16_t render_addr;

    //latches (store next tile info)
    uint8_t at_latch;
    uint8_t lo_bg_latch;
    uint8_t hi_bg_latch;

    //temp variables
    uint8_t nt_byte;
    uint8_t attr_byte;
    uint8_t lo_bg_byte;
    uint8_t hi_bg_byte;

    // true = vertical mirror, false = horizontal mirror
    bool vert_mirror; 

public:
    // holds data to be rendered
    vector<uint8_t> chrrom;
    Frame* frame;

private:
    uint8_t vram_increment();
    uint16_t mirror_addr(uint16_t addr);
    uint8_t ppu_read(uint16_t addr);
    void ppu_write(uint16_t addr, uint8_t data);

public:
    nes_ppu();
    ~nes_ppu();
    virtual void turn_on(nes_system *sys);
    virtual void reset();
    virtual void step_to(uint64_t master_cycle);
    void process_pixel();
    uint8_t cpu_read(uint16_t addr);
    void cpu_write(uint16_t addr, uint8_t data);
    
    void load_chrrom(vector<uint8_t> _chrrom);
    
    void set_mirroring(bool _mirror);
    
    void OAMDMA_w(uint8_t val);

};

#endif