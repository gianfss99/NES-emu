#ifndef MEMORY_HPP
#define MEMORY_HPP


#include <cstdint>
#define MEM_LIMIT 65536

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

class Memory{
private:
    uint8_t *ram;

public:
    Memory();
    //~Memory();
    void mount(int address_space);
    void SET(uint16_t i, uint8_t val);
    uint8_t PEEK(uint16_t i);
    uint8_t operator [](uint16_t i) const;
    uint8_t & operator [](uint16_t i);
    
};

#endif