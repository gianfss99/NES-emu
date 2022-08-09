#ifndef NES_SYSTEM_HPP_
#define NES_SYSTEM_HPP_

#include <vector>
#include "nes_cpu.hpp"
#include "nes_ppu.hpp"
#include "helper.hpp"
#include "nes_memory.hpp"
#include "nes_mapper.hpp"
#include "frame.hpp"
#include <SDL2/SDL.h>

#define PIXEL_SIZE 3

class nes_cpu;
class nes_ppu;
class nes_memory;
class nes_component;
class nes_mapper;
class nes_rom_loader;
class Frame;

class nes_system{
private:
    nes_cpu *_nes_cpu;
    nes_ppu *_nes_ppu;
    nes_memory *_nes_mem;
    Frame *frame;
    std::vector<nes_component*> components;
    nes_mapper* _nes_mapper;
    nes_rom_loader* loader;
    uint64_t master_cycle;   
    bool frame_complete; 

    uint8_t controller[2];

public:
    nes_system();
    void turn_on();
    void step(uint64_t _cycle);
    void on(char* rom);
    
    void run();
    nes_cpu* cpu();
    nes_memory* mem();
    nes_ppu* ppu();
    Frame* get_Frame();
    void frame_completed();
    SDL_Renderer *renderer;
    uint8_t fps = 0x1;

private:
    void init(char* rom);
    void init_sdl();
    SDL_Event event;
    
    SDL_Window *window;
    void draw();
};

#endif