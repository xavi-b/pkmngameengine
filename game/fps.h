#ifndef FPS_H
#define FPS_H

#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

#define FPS 60.0

struct Fps
{
    static const int TickDelay = 300;

    Fps(SDL_Renderer* renderer);
    ~Fps();

    size_t previous   = 0;
    size_t next       = 0;
    size_t accumulate = 0;
    size_t value      = 0;
    bool   tick       = false;

    void draw();

private:
    SDL_Renderer* renderer;
    TTF_Font*     font;
    SDL_Surface*  surface;
    SDL_Texture*  texture;
};

#endif // FPS_H
