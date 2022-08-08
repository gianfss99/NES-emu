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

    }else if(addr < IOREGS){
        switch(addr){
            case 0x4014:
                ppu->OAMDMA_w(val);
                break;
            // default:
            //     cout<<"INVALID WRITE TO REGISTER "<<std::hex<<addr<<endl;
            //     exit(1);
        }

    }else{
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
    else data = ram[addr];
    return data;
}

void nes_memory::SET_CHUNK(uint16_t addr, vector<uint8_t> chunk){
    memcpy(&ram[0]+addr,chunk.data(),chunk.size());
    std::cout<<"CHUNK SET"<<std::endl;
}