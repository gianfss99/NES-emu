#ifndef FRAME_HPP_
#define FRAME_HPP_

#include <vector>
#include <tuple>
#include <iostream>

#define WIDTH   256
#define HEIGHT  240

using namespace std;

class Frame{
public:
    vector<uint8_t> data;
    bool updated = false;
    Frame();
    ~Frame();
    void set_pixel(int x, int y, tuple<uint8_t,uint8_t,uint8_t> &color);
};


#endif