#ifndef FPS_H
#define FPS_H

#include "renderutils.h"

#include <SDL.h>
#include <string>

#define FPS 60.0

struct Fps
{
    static int const TickDelay = 100;

    Fps(SDL_Renderer* renderer);
    ~Fps();

    size_t previous   = 0;
    size_t next       = 0;
    size_t accumulate = 0;
    size_t value      = 0;
    bool   tick       = false;

    void update();
    void draw(RenderSizes rs);

    float tickPercentage() const;

private:
    SDL_Renderer* renderer;
};

#endif // FPS_H
