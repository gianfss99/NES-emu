#include "../inc/frame.hpp"

Frame::Frame(){
    data.resize(WIDTH*HEIGHT*3);
}

void Frame::set_pixel(int x, int y, tuple<uint8_t,uint8_t,uint8_t> color){
    int base = y * 3 * WIDTH + x * 3;
    if(base+2 < data.size()){
        data[base] = get<0>(color);
        data[base+1] = get<1>(color);
        data[base+2] = get<2>(color);
    }
}