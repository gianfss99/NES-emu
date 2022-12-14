#include <cstdio>
#include <iostream>

#include "../inc/nes_system.hpp"



nes_system::nes_system(){
    init_sdl();
    _nes_cpu = new nes_cpu();
    _nes_mem = new nes_memory();
    _nes_ppu = new nes_ppu();
    frame = new Frame(renderer);
    _nes_mapper = nullptr;
    loader = nullptr;
    components.push_back(_nes_cpu);
    components.push_back(_nes_mem);
    components.push_back(_nes_ppu);
    frame_complete = false;
    master_cycle = 0;
}

void nes_system::init(char* rom){
    if(loader != nullptr){
        delete loader;
        loader = nullptr;
    }

    if(_nes_mapper != nullptr){
        delete _nes_mapper;
        _nes_mapper = nullptr;
    }
    loader = new nes_rom_loader(rom);
    _nes_mapper = loader->read_rom();
    _nes_mapper->load_prg(*_nes_mem);
    _nes_mapper->load_ppu(*_nes_ppu);

    
}

void nes_system::on(char* rom){
    init(rom);
    for(auto comp: components) comp->turn_on(this);

}

void nes_system::run(){
    // _nes_cpu->cycle();
    for(;;)
        step(1);
}

void nes_system::step(uint64_t _cycle){
    _nes_cpu->step_to(master_cycle);
    _nes_ppu->step_to(master_cycle);
    master_cycle += _cycle;
    if(frame_complete){
        //std::cout<<std::dec<<(int)fps<<std::endl;
        fps = (fps % 60) + 1;
        draw();
        frame_complete = false;
        frame->updated = false;
    }

    

    SDL_PollEvent(&event);
        switch(event.type){
                case SDL_KEYDOWN:
                    //_nes_mem->controller[0] = 0x0;
                    if(event.key.keysym.sym == SDLK_k) _nes_mem->controller[0] |= 0x01; // A
                    if(event.key.keysym.sym == SDLK_j) _nes_mem->controller[0] |= 0x02; // B
                    if(event.key.keysym.sym == SDLK_v) _nes_mem->controller[0] |= 0x04; // Select
                    if(event.key.keysym.sym == SDLK_b) _nes_mem->controller[0] |= 0x08; // Start
                    if(event.key.keysym.sym == SDLK_w) _nes_mem->controller[0] |= 0x10; // Up
                    if(event.key.keysym.sym == SDLK_s) _nes_mem->controller[0] |= 0x20; // Down
                    if(event.key.keysym.sym == SDLK_a) _nes_mem->controller[0] |= 0x40; // Left
                    if(event.key.keysym.sym == SDLK_d) _nes_mem->controller[0] |= 0x80; // Right
                    //std::cout<<(uint16_t)_nes_mem->controller[0]<<std::endl;
                    break;
                case SDL_KEYUP:
                    if(event.key.keysym.sym == SDLK_k) _nes_mem->controller[0] &= ~(0x01); // A
                    if(event.key.keysym.sym == SDLK_j) _nes_mem->controller[0] &= ~(0x02); // B
                    if(event.key.keysym.sym == SDLK_v) _nes_mem->controller[0] &= ~(0x04); // Select
                    if(event.key.keysym.sym == SDLK_b) _nes_mem->controller[0] &= ~(0x08); // Start
                    if(event.key.keysym.sym == SDLK_w) _nes_mem->controller[0] &= ~(0x10); // Up
                    if(event.key.keysym.sym == SDLK_s) _nes_mem->controller[0] &= ~(0x20); // Down
                    if(event.key.keysym.sym == SDLK_a) _nes_mem->controller[0] &= ~(0x40); // Left
                    if(event.key.keysym.sym == SDLK_d) _nes_mem->controller[0] &= ~(0x80); // Right
                    break;
                case SDL_QUIT:
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    exit(1);
                    break;  
        }
}


nes_cpu* nes_system::cpu(){
    return _nes_cpu;
}

nes_ppu* nes_system::ppu(){
    return _nes_ppu;
}

nes_memory* nes_system::mem(){
    return _nes_mem;
}

Frame* nes_system::get_Frame(){
    return frame;
}

void nes_system::frame_completed(){
    frame_complete = true;
}

void nes_system::init_sdl(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_BATCHING,"1");
    SDL_CreateWindowAndRenderer(WIDTH*PIXEL_SIZE, HEIGHT*PIXEL_SIZE, 0, &window, &renderer);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

void nes_system::draw(){
    // SDL_SetRenderDrawColor(renderer,0,0,0,0);
    // SDL_RenderClear(renderer);
    // if(!frame->updated) {
    //     frame->rects.clear();
    //     return;
    // };
    // for (int y = 0; y < HEIGHT; y++){
    //     for(int x = 0; x < WIDTH; x++){
    //         //create rectangle to create pixel (each px is a 5x5 px rectangle)
    //         SDL_Rect r;
    //         r.x = PIXEL_SIZE*x;
    //         r.y = PIXEL_SIZE*y;
    //         r.w = PIXEL_SIZE;
    //         r.h = PIXEL_SIZE;
    //         int base = y * 3 * WIDTH + x * 3;
    //         SDL_SetRenderDrawColor(renderer,frame->data[base],frame->data[base+1],frame->data[base+2],255);
    //         SDL_RenderFillRect(renderer,&r);
    //         SDL_RenderDrawRect(renderer,&r);
    //         //SDL_RenderDrawPoint(renderer, 5*j, i);
    //     }
    // }
    SDL_RenderDrawRects(renderer,frame->rects.data(),frame->rects.size());
    SDL_RenderPresent(renderer);
    frame->rects.clear();
}