#include "inputs.h"

#include <iostream>

void Inputs::update()
{
    clear();

    Uint8 const* keyState = SDL_GetKeyboardState(NULL);

    if (!keyState)
    {
        return;
    }

    if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A])
    {
        left = true;
        std::cout << "LEFT pressed" << std::endl;
    }
    if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
    {
        right = true;
        std::cout << "RIGHT pressed" << std::endl;
    }
    if (keyState[SDL_SCANCODE_UP] || keyState[SDL_SCANCODE_W])
    {
        up = true;
        std::cout << "UP pressed" << std::endl;
    }
    if (keyState[SDL_SCANCODE_DOWN] || keyState[SDL_SCANCODE_S])
    {
        down = true;
        std::cout << "DOWN pressed" << std::endl;
    }

    if (keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_RETURN2])
    {
        A = true;
        std::cout << "A pressed" << std::endl;
    }
    else if (keyState[SDL_SCANCODE_ESCAPE])
    {
        B = true;
        std::cout << "B pressed" << std::endl;
    }
    else if (keyState[SDL_SCANCODE_I])
    {
        start = true;
        std::cout << "START pressed" << std::endl;
    }
    else if (keyState[SDL_SCANCODE_K])
    {
        select = true;
        std::cout << "SELECT pressed" << std::endl;
    }
    else if (keyState[SDL_SCANCODE_P])
    {
        debug = true;
        std::cout << "DEBUG pressed" << std::endl;
    }

    if (!((left ^ right) ^ (up ^ down)))
    {
        left  = false;
        right = false;
        up    = false;
        down  = false;
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
