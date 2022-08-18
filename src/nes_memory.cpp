#include "../inc/nes_memory.hpp"

nes_memory::nes_memory(){
    ram.resize(MEM_LIMIT);
    _nes_system = nullptr;
    ppu = nullptr;
}

void nes_memory::turn_on(nes_system *sys){
    _nes_system = sys;
    ppu = _nes_system->ppu();
}

void nes_memory::reset(){}

void nes_memory::step_to(uint64_t master_cycle){}

/*
    Set a byte
*/
void nes_memory::SET(uint16_t addr, uint8_t val){
    if(addr < RAM_LIMIT){
        ram[addr] = val;
    }
    else if(addr < PPUREGS){
        
        ppu->cpu_write(addr & 0x7,val);           
        //std::cout<<std::hex<<"PPUWRITE: "<<(uint16_t)val<<"@"<<addr<<std::endl;

    }else if(addr == 0x4014){
        
        ppu->OAMDMA_w(val);
    }

    else if(addr == 0x4016 || addr == 0x4017){
        controller_state[addr & 0x1] = controller[addr & 0x1];
        

    }
    
    else{
        ram[addr] = val;
    }
}

/*
    Retrieve a byte
*/
uint8_t nes_memory::PEEK(uint16_t addr){
    uint8_t data = 0x00;
    if(addr < RAM_LIMIT){
        data = ram[addr];
    }
    else if(addr < PPUREGS){
        data = ppu->cpu_read(addr & 0x7);
        //std::cout<<std::hex<<"PPUREAD: "<<addr<<std::endl;
    }
    else if(addr == 0x4016 || addr == 0x4017){
        data = (controller_state[addr & 0x1] & 0x1) > 0;
        controller_state[addr & 0x1] >>= 1;
        //std::cout<<std::hex<<(uint16_t)controller_state[addr & 0x1]<<std::endl;

        //std::cout<<"Input read"<<std::endl;
    }
    else data = ram[addr];
    return data;
}

void nes_memory::SET_CHUNK(uint16_t addr, vector<uint8_t> chunk){
    memcpy(&ram[0]+addr,chunk.data(),chunk.size());
    //std::cout<<"CHUNK SET"<<std::endl;
}