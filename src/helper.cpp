#include "../inc/helper.hpp"

int get_file_size(FILE* fileptr){
    fseek(fileptr,0,SEEK_END);
    int filelen = ftell(fileptr);
    rewind(fileptr);
    return filelen;
}

uint8_t reverse(uint8_t b){
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}