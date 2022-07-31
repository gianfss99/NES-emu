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
        switch(addr){
            case 0x2000:
                ppu->PPUCTRL_w(val);
                break;
            case 0x2001:
                ppu->PPUMASK_w(val);
                break;
            case 0x2003:
                ppu->OAMADDR() = val;
                break;
            case 0x2004:
                ppu->OAMDATA() = val;
                break;
            case 0x2005:
               // ppu->PPUSCROLL() = val;
                break;
            case 0x2006:
                ppu->PPUADDR_w(val);
                break;
            case 0x2007:
                ppu->write_data(val);
                break;
            default:
                cout<<"INVALID WRITE TO REGISTER "<<std::hex<<0x2000+(addr%0x8)<<endl;
                exit(1);
            
        }
    }else if(addr < IOREGS){
        switch(addr){
            case 0x4014:
                ppu->OAMDMA_w(val);
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
        switch(addr){
            case 0x2002:
                return ppu->PPUSTATUS_r();
            case 0x2004:
                return ppu->OAMDATA();
            case 0x2007:
                return ppu->read_data();
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