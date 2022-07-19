#include "helper.hpp"

int get_file_size(FILE* fileptr){
    fseek(fileptr,0,SEEK_END);
    int filelen = ftell(fileptr);
    //std::cout<<"filelen: "<<filelen<<std::endl;
    //if(filelen>MAX_PROGRAM_SIZE) return -1;
    rewind(fileptr);
    return filelen;
}