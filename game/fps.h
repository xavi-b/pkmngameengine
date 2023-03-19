#ifndef FPS_H
#define FPS_H

#include "renderutils.h"
#include <string>
#include <SDL.h>

#define FPS 60.0

struct Fps
{
    static const int TickDelay = 150;

    Fps(SDL_Renderer* renderer);
    ~Fps();

    size_t previous   = 0;
    size_t next       = 0;
    size_t accumulate = 0;
    size_t value      = 0;
    bool   tick       = false;

    void draw(RenderSizes rs);

private:
    SDL_Renderer* renderer;
};

#endif // FPS_H
