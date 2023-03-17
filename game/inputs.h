#ifndef INPUTS_H
#define INPUTS_H

#include <SDL.h>
#include "fps.h"

struct Inputs
{
    void update(SDL_Event event);
    void clear();

    bool A      = false;
    bool B      = false;
    bool up     = false;
    bool down   = false;
    bool left   = false;
    bool right  = false;
    bool start  = false;
    bool select = false;
};

#endif // ITEM_H
