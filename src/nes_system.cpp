#include <cstdio>
#include <iostream>

#include "../inc/nes_system.hpp"


nes_system::nes_system(){
    _nes_cpu = new nes_cpu();
    _nes_mem = new nes_memory();
    _nes_ppu = new nes_ppu();
    _nes_mapper = nullptr;
    loader = nullptr;
    components.push_back(_nes_cpu);
    components.push_back(_nes_mem);
    components.push_back(_nes_ppu);
}

void nes_system::init(char* rom){
    if(loader != nullptr){
        delete loader;
        loader = nullptr;
    }

    if(_nes_mapper != nullptr){
        delete _nes_mapper;
        _nes_mapper = nullptr;
    }
    loader = new nes_rom_loader(rom);
    _nes_mapper = loader->read_rom();
    _nes_mapper->load_prg(*_nes_mem);
}

void nes_system::on(char* rom){
    init(rom);
    for(auto comp: components) comp->turn_on(this);

}

void nes_system::run(){
    _nes_cpu->cycle();
}

nes_cpu* nes_system::cpu(){
    return _nes_cpu;
}

nes_ppu* nes_system::ppu(){
    return _nes_ppu;
}

nes_memory* nes_system::mem(){
    return _nes_mem;
}