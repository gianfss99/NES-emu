#include "../inc/helper.hpp"

int get_file_size(FILE* fileptr){
    fseek(fileptr,0,SEEK_END);
    int filelen = ftell(fileptr);
    rewind(fileptr);
    return filelen;
}
