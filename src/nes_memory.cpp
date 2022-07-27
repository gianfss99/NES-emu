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

void nes_memory::step_to(int _cycle){}

/*
    Set a byte
*/
void nes_memory::SET(uint16_t addr, uint8_t val){
    if(addr < RAM_LIMIT){
        ram[addr] = val;
    }
    else if(addr < PPUREGS){
        switch(addr % 0x8){
            case 0x0:
                ppu->PPUCTRL() = val;
                break;
            case 0x1:
                ppu->PPUMASK() = val;
                break;
            case 0x3:
                ppu->OAMADDR() = val;
                break;
            case 0x4:
                ppu->OAMDATA() = val;
                break;
            case 0x5:
                ppu->PPUSCROLL() = val;
                break;
            case 0x6:
                ppu->update_PPUADDR(val);
                break;
            case 0x7:
                ppu->PPUDATA() = val;
                break;
            default:
                cout<<"INVALID WRITE TO REGISTER "<<std::hex<<0x2000+(addr%0x8)<<endl;
                exit(1);
            
        }
    }else if(addr < IOREGS){
        switch(addr % 0x20){
            case 0x14:
                ppu->OAMDMA() = val;
                break;
            default:
                cout<<"INVALID WRITE TO REGISTER "<<std::hex<<addr<<endl;
                exit(1);
        }

    }else{
        ram[addr] = val;
    }
}

/*
    Retrieve a byte
*/
uint8_t nes_memory::PEEK(uint16_t addr){
    if(addr < RAM_LIMIT){
        return ram[addr];
    }
    else if(addr < PPUREGS){
        switch(addr % 0x8){
            case 0x2:
                return ppu->PPUSTATUS();
            case 0x4:
                return ppu->OAMDATA();
            case 0x7:
                return ppu->PPUDATA();
            default:
                cout<<"INVALID READ FROM REGISTER "<<std::hex<<0x2000+(addr%0x8)<<endl;
                exit(1);
        }
    }
    else return ram[addr];
}

void nes_memory::SET_CHUNK(uint16_t addr, vector<uint8_t> chunk){
    memcpy(&ram[0]+addr,chunk.data(),chunk.size());
    std::cout<<"CHUNK SET"<<std::endl;
}