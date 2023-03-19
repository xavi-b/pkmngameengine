#include "inputs.h"

#include <iostream>

void Inputs::update(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
        std::cout << "SDL_KEYDOWN" << std::endl;
        switch (event.key.keysym.sym)
        {
            std::cout << event.key.keysym.sym << " pressed" << std::endl;
        case SDLK_LEFT:
        case SDLK_a:
            left |= true;
            std::cout << "LEFT pressed" << std::endl;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            right |= true;
            std::cout << "RIGHT pressed" << std::endl;
            break;
        case SDLK_UP:
        case SDLK_w:
            up |= true;
            std::cout << "UP pressed" << std::endl;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            down |= true;
            std::cout << "DOWN pressed" << std::endl;
            break;
        case SDLK_RETURN:
        case SDLK_RETURN2:
            A |= true;
            std::cout << "A pressed" << std::endl;
            break;
        case SDLK_ESCAPE:
            B |= true;
            std::cout << "B pressed" << std::endl;
            break;
        case SDLK_i:
            start |= true;
            std::cout << "START pressed" << std::endl;
            break;
        case SDLK_k:
            select |= true;
            std::cout << "SELECT pressed" << std::endl;
            break;
        case SDLK_p:
            debug |= true;
            std::cout << "DEBUG pressed" << std::endl;
            break;
        default:
            break;
        }
    }
}

void Inputs::clear()
{
    A      = false;
    B      = false;
    up     = false;
    down   = false;
    left   = false;
    right  = false;
    start  = false;
    select = false;
    debug  = false;
}
