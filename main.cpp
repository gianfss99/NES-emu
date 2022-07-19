#include "system.hpp"

int main(int argc, char* argv[]){
    System nes_system = System();
    std::string file = "tests/nestest.nes";
    std::cout.fill('0');
    nes_system.on(file.c_str());
    nes_system.run();
}

