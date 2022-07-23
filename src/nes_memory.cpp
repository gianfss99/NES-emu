#include "../inc/nes_memory.hpp"
#include <cstdlib>
#include <cstring>

nes_memory::nes_memory(){
    ram = (uint8_t*)calloc(MEM_LIMIT,sizeof(uint8_t));
    _nes_system = nullptr;
}

void nes_memory::turn_on(nes_system *sys){
    _nes_system = sys;
}

void nes_memory::reset(){}

void nes_memory::step_to(int _cycle){}

/*
    Set a byte
*/
void nes_memory::SET(uint16_t addr, uint8_t val){
    ram[addr] = val;
}

/*
    Retrieve a byte
*/
uint8_t nes_memory::PEEK(uint16_t addr){
    return ram[addr];
}

void nes_memory::SET_CHUNK(uint16_t addr, vector<uint8_t> chunk){
    memcpy(ram+addr,chunk.data(),chunk.size());
    std::cout<<"CHUNK SET"<<std::endl;
}