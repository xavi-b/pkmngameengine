#include "fps.h"

#include <iostream>

Fps::Fps(SDL_Renderer* renderer)
    : renderer(renderer)
{
    previous = SDL_GetTicks();
    next     = SDL_GetTicks() + TickDelay;

    font = TTF_OpenFont("resources/Fonts/power clear.ttf", 24);
    if (font == NULL)
        std::cerr << TTF_GetError() << std::endl;
}

Fps::~Fps()
{
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Fps::draw()
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

    SDL_Color color = {255, 255, 255};

    auto fpsStr = "FPS: " + std::to_string(value);
    surface     = TTF_RenderText_Solid(font, fpsStr.c_str(), color);

    texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
