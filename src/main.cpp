#include "../inc/nes_system.hpp"
#include <string>
#include <cstring>
#include <SDL2/SDL.h>

SDL_Event event;
SDL_Renderer *renderer;
SDL_Window *window;
Frame* frame;

vector<tuple<uint8_t,uint8_t,uint8_t>> SYSTEM_PALETTE = {
    make_tuple(0x80, 0x80, 0x80), make_tuple(0x00, 0x3D, 0xA6), make_tuple(0x00, 0x12, 0xB0), make_tuple(0x44, 0x00, 0x96), make_tuple(0xA1, 0x00, 0x5E),
    make_tuple(0xC7, 0x00, 0x28), make_tuple(0xBA, 0x06, 0x00), make_tuple(0x8C, 0x17, 0x00), make_tuple(0x5C, 0x2F, 0x00), make_tuple(0x10, 0x45, 0x00),
    make_tuple(0x05, 0x4A, 0x00), make_tuple(0x00, 0x47, 0x2E), make_tuple(0x00, 0x41, 0x66), make_tuple(0x00, 0x00, 0x00), make_tuple(0x05, 0x05, 0x05),
    make_tuple(0x05, 0x05, 0x05), make_tuple(0xC7, 0xC7, 0xC7), make_tuple(0x00, 0x77, 0xFF), make_tuple(0x21, 0x55, 0xFF), make_tuple(0x82, 0x37, 0xFA),
    make_tuple(0xEB, 0x2F, 0xB5), make_tuple(0xFF, 0x29, 0x50), make_tuple(0xFF, 0x22, 0x00), make_tuple(0xD6, 0x32, 0x00), make_tuple(0xC4, 0x62, 0x00),
    make_tuple(0x35, 0x80, 0x00), make_tuple(0x05, 0x8F, 0x00), make_tuple(0x00, 0x8A, 0x55), make_tuple(0x00, 0x99, 0xCC), make_tuple(0x21, 0x21, 0x21),
    make_tuple(0x09, 0x09, 0x09), make_tuple(0x09, 0x09, 0x09), make_tuple(0xFF, 0xFF, 0xFF), make_tuple(0x0F, 0xD7, 0xFF), make_tuple(0x69, 0xA2, 0xFF),
    make_tuple(0xD4, 0x80, 0xFF), make_tuple(0xFF, 0x45, 0xF3), make_tuple(0xFF, 0x61, 0x8B), make_tuple(0xFF, 0x88, 0x33), make_tuple(0xFF, 0x9C, 0x12),
    make_tuple(0xFA, 0xBC, 0x20), make_tuple(0x9F, 0xE3, 0x0E), make_tuple(0x2B, 0xF0, 0x35), make_tuple(0x0C, 0xF0, 0xA4), make_tuple(0x05, 0xFB, 0xFF),
    make_tuple(0x5E, 0x5E, 0x5E), make_tuple(0x0D, 0x0D, 0x0D), make_tuple(0x0D, 0x0D, 0x0D), make_tuple(0xFF, 0xFF, 0xFF), make_tuple(0xA6, 0xFC, 0xFF),
    make_tuple(0xB3, 0xEC, 0xFF), make_tuple(0xDA, 0xAB, 0xEB), make_tuple(0xFF, 0xA8, 0xF9), make_tuple(0xFF, 0xAB, 0xB3), make_tuple(0xFF, 0xD2, 0xB0),
    make_tuple(0xFF, 0xEF, 0xA6), make_tuple(0xFF, 0xF7, 0x9C), make_tuple(0xD7, 0xE8, 0x95), make_tuple(0xA6, 0xED, 0xAF), make_tuple(0xA2, 0xF2, 0xDA),
    make_tuple(0x99, 0xFF, 0xFC), make_tuple(0xDD, 0xDD, 0xDD), make_tuple(0x11, 0x11, 0x11), make_tuple(0x11, 0x11, 0x11),
};

void show_tile(vector<uint8_t> &chrrom,uint16_t bank,uint16_t tile_no){
    if(bank>1) return;

     
    bank = bank * 0x1000;
    int tile = bank + tile_no * 16;
    for(int y = 0; y<8; y++){
        uint8_t msb = chrrom[tile+y];
        uint8_t lsb = chrrom[tile+y+8];
        for(int x = 0; x<8; x++){
            uint8_t val = (msb & 0x80)>>6 | (lsb & 0x80)>>7;
            msb <<= 1;
            lsb <<= 1;
            tuple<uint8_t,uint8_t,uint8_t> color;
            switch(val){
                case 0:
                    color = SYSTEM_PALETTE[0x01];
                    break;
                case 1:
                    color = SYSTEM_PALETTE[0x23];
                    break;
                case 2:
                    color = SYSTEM_PALETTE[0x27];
                    break;
                case 3:
                    color = SYSTEM_PALETTE[0x30];
                    break;
            }
            frame->set_pixel(x,y,color);
        }
    }

}

void init_sdl(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WIDTH*2, HEIGHT*2, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

void draw(){
    SDL_SetRenderDrawColor(renderer,0,0,0,0);
    SDL_RenderClear(renderer);
    for (int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
            //create rectangle to create pixel (each px is a 5x5 px rectangle)
            SDL_Rect r;
            r.x = 2*x;
            r.y = 2*y;
            r.w = 2;
            r.h = 2;
            int base = y * 3 * WIDTH + x * 3;
            SDL_SetRenderDrawColor(renderer,frame->data[base],frame->data[base+1],frame->data[base+2],255);
            SDL_RenderFillRect(renderer,&r);
            SDL_RenderDrawRect(renderer,&r);
            //SDL_RenderDrawPoint(renderer, 5*j, i);
        }
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]){
    init_sdl();
    nes_system _nes_system = nes_system();
    std::cout<<"HELLO"<<std::endl;
    std::string str = "tests/pacman.nes";
    char* file = strcpy(new char[str.length() + 1], str.c_str());
    std::cout.fill('0');
    _nes_system.on(file);
    frame = new Frame();
    show_tile(_nes_system.ppu()->chrrom,0,1);
    draw();
    //_nes_system.run();
    bool quit = false;
    while(!quit){
        if(SDL_PollEvent(&event)){
            switch(event.type){
                    case SDL_QUIT:
                        quit = true;
                        break;  
                    case SDL_WINDOWEVENT:

                        switch (event.window.event) {
                            case SDL_WINDOWEVENT_CLOSE:   // exit game
                                quit = true;
                                break;
                            default:
                                break;
                        }

                        break;
                    
                    default:
                        break;
            }
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

