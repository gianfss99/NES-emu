#include "../inc/nes_system.hpp"
#include <string>
#include <cstring>
#include <SDL2/SDL.h>


int main(int argc, char* argv[]){
    if(argc < 2) return 0;

    nes_system _nes_system = nes_system();
    std::string str = "tests/ice_climbers.nes";
    char* file = strcpy(new char[str.length() + 1], str.c_str());
    std::cout.fill('0');
    _nes_system.on(argv[1]);
    _nes_system.run();
}

