#ifndef NES_COMPONENT_HPP_
#define NES_COMPONENT_HPP_

#include "nes_system.hpp"

class nes_system;

class nes_component{
public:
    virtual void turn_on(nes_system *sys) = 0;
    virtual void reset() = 0;
    virtual void step_to(uint64_t master_cycle) = 0;

};
#endif