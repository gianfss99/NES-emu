#ifndef FRAME_HPP_
#define FRAME_HPP_

#include <vector>
#include <tuple>
#include <iostream>
#include <SDL2/SDL.h>

#define WIDTH   256
#define HEIGHT  240
#define PIXEL_SIZE 3


using namespace std;

class Frame{
public:
    SDL_Renderer *renderer;
    vector<uint8_t> data;
    bool updated = false;
    vector<SDL_Rect> rects;
    Frame(SDL_Renderer *_renderer);
    ~Frame();
    void set_pixel(int x, int y, tuple<uint8_t,uint8_t,uint8_t> &color);
};


#endif