#ifndef NES_MAPPER_HPP_
#define NES_MAPPER_HPP_

#include "nes_system.hpp"
#include "nes_memory.hpp"
#include "nes_ppu.hpp"
#include <fstream>
#include <vector>

using namespace std;

class nes_system;
class nes_memory;
class nes_ppu;

struct mapper_flags{
    bool vert_mirror;
};

/*
    base class for all mappers (just good practice, probably only implementing mapper 0 (nrom))
*/
class nes_mapper{
public:
    virtual void load_ppu(nes_ppu &ppu) = 0;
    virtual void load_prg(nes_memory &mem) = 0;
    virtual void load_trainer(nes_memory &mem) = 0;
};

class mapper_nrom: public nes_mapper{
private:
    vector<uint8_t> prgrom;
    vector<uint8_t> chrrom;
    vector<uint8_t> trainer;
    mapper_flags flags;

public:
    mapper_nrom(vector<uint8_t> _prgrom, vector<uint8_t> _chrrom, vector<uint8_t> trainer, mapper_flags _flags);
    virtual void load_ppu(nes_ppu &ppu);
    virtual void load_prg(nes_memory &mem);
    virtual void load_trainer(nes_memory &mem);
};

class nes_rom_loader{
private:
    uint8_t header[16];
    mapper_flags flags;
    ifstream rom;
    vector<uint8_t> prgrom;
    vector<uint8_t> chrrom;
    vector<uint8_t> trainer;
    bool iNES;
    bool NES20;

public:
    nes_rom_loader(char *filename);
    nes_mapper* read_rom();
};

#endif