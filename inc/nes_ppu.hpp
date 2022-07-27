#ifndef NES_PPU_HPP_
#define NES_PPU_HPP_

#include "nes_system.hpp"
#include "nes_cpu.hpp"
#include "nes_component.hpp"
#include "nes_mapper.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>

#define VRAM_LIMIT      0x800
#define PALETTE_LIMIT   0x20
#define OAM_LIMIT       0x100

using namespace std;

class nes_system;
class nes_cpu;

struct large_reg{
    uint8_t value[2] = {};
    bool hi = true;
};

struct PPU_REGS{
    uint8_t PPUCTRL;
    uint8_t PPUMASK;
    uint8_t PPUSTATUS;
    uint8_t OAMADDR;
    uint8_t OAMDATA;
    uint8_t PPUSCROLL;
    large_reg PPUADDR;
    uint8_t PPUDATA;
    uint8_t OAMDMA;
};

class nes_ppu: public nes_component{
private:
    // PPU registers
    PPU_REGS registers;
    bool vert_mirror; //true=vertical mirror, false=horizontal mirror

    nes_system *_nes_system;
    nes_cpu *cpu;

    vector<uint8_t> chrrom;
    vector<uint8_t> palette_table;
    vector<uint8_t> vram;
    vector<uint8_t> oam_data;

    uint8_t vram_increment();
    uint8_t increment_addr();
    void set_PPUADDR(uint16_t val);
    void inc_PPUADDR();
    uint16_t get_PPUADDR();
    void reset_PPUADDR();

public:
    nes_ppu();
    ~nes_ppu();
    virtual void turn_on(nes_system *sys);
    virtual void reset();
    virtual void step_to(int _cycle);
    void load_chrrom(vector<uint8_t> _chrrom);

    void set_mirroring(bool _mirror);

    uint8_t& PPUCTRL();
    uint8_t& PPUMASK();
    uint8_t& PPUSTATUS();
    uint8_t& OAMADDR();
    uint8_t& OAMDATA();
    uint8_t& PPUSCROLL();
    uint8_t& PPUDATA();
    uint8_t& OAMDMA();
    uint8_t read_data();

    void update_PPUADDR(uint8_t val);

};

#endif