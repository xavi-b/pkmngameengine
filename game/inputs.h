#ifndef INPUTS_H
#define INPUTS_H

#include <SDL.h>

struct Inputs
{
    void update();
    void clear();

    bool A      = false;
    bool B      = false;
    bool up     = false;
    bool down   = false;
    bool left   = false;
    bool right  = false;
    bool start  = false;
    bool select = false;
    bool debug  = false;
};

#endif // ITEM_H
