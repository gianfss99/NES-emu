#ifndef NES_PPU_HPP_
#define NES_PPU_HPP_

#include "nes_system.hpp"
#include "nes_component.hpp"

#define VRAM_LIMIT      0x800
#define PALETTE_LIMIT   0x20
#define OAM_LIMIT       0x100

class nes_system;

class nes_ppu: public nes_component{
private:
    // PPU registers
    uint8_t PPUCTRL;
    uint8_t PPUMASK;
    uint8_t OAMADDR;
    uint8_t OAMDATA;
    uint8_t PPUSCROLL;
    uint8_t PPUADDR;
    uint8_t PPUDATA;
    uint8_t OAMDMA;

    uint8_t *chrrom;
    uint8_t *palette_table;
    uint8_t *vram;
    uint8_t *oam_data;

public:
    virtual void turn_on(nes_system *sys);
    virtual void reset();
    virtual void step_to(int _cycle);
};

#endif