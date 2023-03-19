#include "fps.h"

#include <iostream>

Fps::Fps(SDL_Renderer* renderer)
    : renderer(renderer)
{
    previous = SDL_GetTicks();
    next     = SDL_GetTicks() + TickDelay;
}

Fps::~Fps()
{
}

void Fps::draw(RenderSizes rs)
{
    accumulate++;

    if (SDL_GetTicks() >= next)
    {
        value = accumulate;

        accumulate = 0;

        next = SDL_GetTicks() + TickDelay;

        tick = true;
    }
    else
    {
        tick = false;
    }

    previous = SDL_GetTicks();

    auto      fpsStr  = "FPS: " + std::to_string(value);
    SDL_Color color   = {255, 255, 255};
    SDL_Color bgColor = {127, 127, 127};
    RenderUtils::drawText(renderer, rs, fpsStr, color, bgColor, 0, 0);
}
