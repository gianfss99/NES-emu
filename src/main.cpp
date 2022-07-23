#include "../inc/nes_system.hpp"
#include <string>
#include <cstring>

int main(int argc, char* argv[]){
    nes_system _nes_system = nes_system();
    std::cout<<"HELLO"<<std::endl;
    std::string str = "tests/nestest.nes";
    char* file = strcpy(new char[str.length() + 1], str.c_str());
    std::cout.fill('0');
    _nes_system.on(file);
    _nes_system.run();
}

