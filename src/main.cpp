#include "../inc/nes_system.hpp"
#include <string>
#include <cstring>
#include <SDL2/SDL.h>



// void show_tile(vector<uint8_t> &chrrom,uint16_t bank,uint16_t tile_no){
//     if(bank>1) return;

     
//     bank = bank * 0x1000;
    
//     int tile = bank + tile_no * 16;
//     int base_x = (tile % 512) / 2;
//     int base_y = 8*(tile / 512);
//     for(int y = 0; y<8; y++){
//         uint8_t msb = chrrom[tile+y];
//         uint8_t lsb = chrrom[tile+y+8];
//         for(int x = 0; x<8; x++){
//             uint8_t val = (msb & 0x80)>>6 | (lsb & 0x80)>>7;
//             msb <<= 1;
//             lsb <<= 1;
//             tuple<uint8_t,uint8_t,uint8_t> color;
//             switch(val){
//                 case 0:
//                     color = SYSTEM_PALETTE[0x01];
//                     break;
//                 case 1:
//                     color = SYSTEM_PALETTE[0x23];
//                     break;
//                 case 2:
//                     color = SYSTEM_PALETTE[0x27];
//                     break;
//                 case 3:
//                     color = SYSTEM_PALETTE[0x30];
//                     break;
//             }
//             frame->set_pixel(base_x+x,base_y+y,color);
//         }
//     }

// }

// void init_sdl(){
//     SDL_Init(SDL_INIT_VIDEO);
//     SDL_CreateWindowAndRenderer(WIDTH*5, HEIGHT*5, 0, &window, &renderer);
//     SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
//     SDL_RenderClear(renderer);
//     SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
// }

// void draw(){
//     SDL_SetRenderDrawColor(renderer,0,0,0,0);
//     SDL_RenderClear(renderer);
//     for (int y = 0; y < HEIGHT; y++){
//         for(int x = 0; x < WIDTH; x++){
//             //create rectangle to create pixel (each px is a 5x5 px rectangle)
//             SDL_Rect r;
//             r.x = 5*x;
//             r.y = 5*y;
//             r.w = 5;
//             r.h = 5;
//             int base = y * 3 * WIDTH + x * 3;
//             SDL_SetRenderDrawColor(renderer,frame->data[base],frame->data[base+1],frame->data[base+2],255);
//             SDL_RenderFillRect(renderer,&r);
//             SDL_RenderDrawRect(renderer,&r);
//             //SDL_RenderDrawPoint(renderer, 5*j, i);
//         }
//     }
//     SDL_RenderPresent(renderer);
// }

int main(int argc, char* argv[]){
    nes_system _nes_system = nes_system();
    std::cout<<"HELLO"<<std::endl;
    std::string str = "tests/donkeykong.nes";
    char* file = strcpy(new char[str.length() + 1], str.c_str());
    std::cout.fill('0');
    _nes_system.on(file);
    _nes_system.run();
    // frame = new Frame();

    // for(int i = 0; i<20*13-5; i++)
    //     show_tile(_nes_system.ppu()->chrrom,0,i);
    // draw();
    // //_nes_system.run();
    // bool quit = false;
    // while(!quit){
    //     SDL_PollEvent(&event);
    //     switch(event.type){
    //             case SDL_QUIT:
    //                 quit = true;
    //                 break;  
    //     }
    // }
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);

    // SDL_Quit();
}

