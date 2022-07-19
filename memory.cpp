#include "memory.hpp"
#include <cstdlib>

Memory::Memory(){
    
}

void Memory::mount(int address_space){
    ram = (uint8_t *)calloc(address_space,sizeof(uint8_t));
}

void Memory::SET(uint16_t i, uint8_t val){
    ram[i] = val;
}

uint8_t Memory::PEEK(uint16_t i){
    return ram[i];
}

uint8_t Memory::operator [](uint16_t i) const{
    return ram[i];
}

uint8_t & Memory::operator [](uint16_t i){
    return ram[i];
}