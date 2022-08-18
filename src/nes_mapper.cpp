#include "../inc/nes_mapper.hpp"

mapper_nrom::mapper_nrom(vector<uint8_t> _prgrom, vector<uint8_t> _chrrom, vector<uint8_t> _trainer, mapper_flags _flags):
    prgrom(_prgrom),chrrom(_chrrom),trainer(_trainer),flags(_flags){}

void mapper_nrom::load_ppu(nes_ppu &ppu){
    ppu.set_mirroring(flags.vert_mirror);
    ppu.load_chrrom(chrrom);
}

void mapper_nrom::load_prg(nes_memory &mem){
    mem.SET_CHUNK(0x8000,prgrom);
    if(prgrom.size()==0x4000)
        mem.SET_CHUNK(0xC000,prgrom);
}

void mapper_nrom::load_trainer(nes_memory &mem){
    if(trainer.size() != 0)
        mem.SET_CHUNK(0x7000,trainer);
}

nes_rom_loader::nes_rom_loader(char *filename){
    rom.open(filename,ifstream::binary);
    iNES = false;
    NES20 = false;
    flags = {};
}

nes_mapper* nes_rom_loader::read_rom(){
    rom.read((char*)header,16);

    if(header[0]=='N' && header[1]=='E' && header[2]=='S' && header[3]==0x1A)
        iNES = true;
    //does not handle NES2.0 Format
    if(iNES && (header[7]&0x0C)==0x08){
        NES20 = true;
        exit(1);
    }

    if(!iNES) exit(1);

    prgrom.resize(16384*(int)header[4]);
    chrrom.resize(8192*(int)header[5]);


    flags.vert_mirror = (header[6] & 0x01);

    if((header[6] & 0x04) == 0x04){
        trainer.resize(512);
        rom.read((char*)trainer.data(),trainer.size());
    }

    rom.read((char*)prgrom.data(),prgrom.size());
    rom.read((char*)chrrom.data(),chrrom.size());

    uint8_t mapper_id = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);
    nes_mapper* mapper;
    switch(mapper_id){
        case 0:
            mapper = new mapper_nrom(prgrom,chrrom,trainer,flags);
            break;
    }
    rom.close();

    return mapper;
}

