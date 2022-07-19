#include <cstdio>
#include <iostream>

#include "system.hpp"


System::System(){
    nes_cpu = Cpu();
    mapper = 0;
    persistent_memory = false;
}

void System::on(const char* rom){
    read_rom(rom);

}

void System::run(){
    nes_cpu.cycle();
}

void System::load_program(FILE* fileptr, int size, int start){
    bool mirror = (size == 16384);
    uint8_t val;
    for(int i = 0; i<size; i++){
        val = fgetc(fileptr);
        nes_cpu._mem.SET(i+start,val);
        if(mirror)
            nes_cpu._mem.SET(i+start+size,val);
    }
}

void System::load_ppu(FILE* fileptr, int size, int start){
    for(int i = 0; i<size; i++){
        //nes_cpu._mem.SET(i+start,chrrom[i]);
    }
}

int System::read_rom(const char* rom){
    bool iNESFormat = false;
    bool NES20Format = false;
    bool trainer = false;
    int PRGROM = 0;
    int CHRROM = 0;


    FILE *fileptr;
    int filelen;
    
    fileptr = fopen(rom, "r");
    filelen = get_file_size(fileptr);

    //read 16-byte header
    uint8_t header[16];
    for(int i = 0; i<16; i++){
        header[i] = fgetc(fileptr);
    }

    //check file format
    if(header[0]=='N' && header[1]=='E' && header[2]=='S' && header[3]==0x1A)
        iNESFormat = true;
    
    //does not handle NES2.0 Format
    if(iNESFormat && (header[7]&0x0C)==0x08){
        NES20Format = true;
        exit(1);
    }

    // invalid file format
    if(!iNESFormat){
        std::cout<<"Incorrect File Format"<<std::endl;
        exit(1);
    }

    PRGROM = 16384*(int)header[4];
    CHRROM = 8192*(int)header[5];

    mapper = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);

    //flag6
    persistent_memory = ((header[6] & 0x02) >> 1);
    trainer = ((header[6] & 0x04) >> 2);
    //next bit = ignore mirroring control; instead provide four-screen vram
    
    if(trainer)
        load_program(fileptr,512,0x7000);

    // loads PRGROM @ address $8000
    load_program(fileptr,PRGROM,0x8000);

    //load_ppu(fileptr,CHRROM,0x2000)

    return 1;

}