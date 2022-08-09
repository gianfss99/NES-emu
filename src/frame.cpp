#include "../inc/frame.hpp"

Frame::Frame(SDL_Renderer *_renderer){
    data.resize(WIDTH*HEIGHT*3);
    renderer = _renderer;
}

void Frame::set_pixel(int x, int y, tuple<uint8_t,uint8_t,uint8_t> &color){
    
    if(x < 0 || x > 255 || y < 0 || y > 239) return;
    int base = y * 3 * WIDTH + x * 3;
    if(base+2 < data.size()){
        if(data[base]!=get<0>(color) || data[base+1]!=get<1>(color) ||data[base+2]!=get<2>(color)) updated = true;
        //else return;
        data[base] = get<0>(color);
        data[base+1] = get<1>(color);
        data[base+2] = get<2>(color);
        SDL_Rect r;
        r.x = PIXEL_SIZE*x;
        r.y = PIXEL_SIZE*y;
        r.w = PIXEL_SIZE;
        r.h = PIXEL_SIZE;
        SDL_SetRenderDrawColor(renderer,data[base],data[base+1],data[base+2],255);
        SDL_RenderFillRect(renderer,&r);
        rects.push_back(r);
    }
}