#include "fps.h"

#include <iostream>

Fps::Fps(SDL_Renderer* renderer) : renderer(renderer)
{
    previous = SDL_GetTicks();
    next     = SDL_GetTicks() + TickDelay;
}

Fps::~Fps()
{
}

void Fps::update()
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
}

void Fps::draw(RenderSizes rs)
{
    auto fpsStr = "FPS: " + std::to_string(value);
    RenderUtils::drawWhiteText(renderer, rs, fpsStr, 24, 0, 0);
}

float Fps::tickPercentage() const
{
    return (1.0 - 1.0 * (next - previous) / TickDelay);
}
