#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "cpu.hpp"
#include "helper.hpp"

class System{
private:
    Cpu nes_cpu;
    int mapper;
    bool persistent_memory;
public:
    System();
    void master_cycle();
    void on(const char* rom);
    void run();
private:
    int read_rom(const char* rom);
    void load_program(FILE* fileptr, int size, int start);
    void load_ppu(FILE* fileptr, int size, int start);

};

#endif