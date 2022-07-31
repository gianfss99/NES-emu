#ifndef MEMORY_HPP_
#define MEMORY_HPP_

#include <cstdint>
#include "nes_system.hpp"
#include "nes_component.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>

using namespace std;

#define MEM_LIMIT 0x10000
#define RAM_LIMIT 0x2000
#define PPUREGS 0x4000
#define IOREGS 0x4020


/*
    CPU Memory Mapping:
    Addr range      Size        Device
    $0000-$07FF     $0800       2KB internal RAM
    $0800-$0FFF     $0800       Mirrors of $0000-$07FF
    $1000-$17FF     $0800       Mirrors of $0000-$07FF
    $1800-$1FFF     $0800       Mirrors of $0000-$07FF
    $2000-$2007     $0008       NES PPU Registers
    $2008-$3FFF     $1FF8       Mirrors of $2000-$2007 (repeats every 8 bytes)
    $4000-$4017     $0018       NES APU and I/O Registers
    $4018-$401F     $0008       APU and I/O functionality that is normally disabled
    $4020-$FFFF     $BFEO       Cartridge space: PRG ROM, PRG RAM, and mapper registers
*/
class nes_system;
class nes_ppu;

class nes_memory: public nes_component{
private:
    vector<uint8_t> ram;
    nes_system *_nes_system;
    nes_ppu *ppu;

public:
    nes_memory();
    ~nes_memory();
    void SET(uint16_t addr, uint8_t val);
    uint8_t PEEK(uint16_t addr);
    void SET_CHUNK(uint16_t addr, vector<uint8_t> chunk);
    virtual void turn_on(nes_system *sys);
    virtual void reset();
    virtual void step_to(uint64_t master_cycle);
};

#endif