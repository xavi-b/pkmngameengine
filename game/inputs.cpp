#include "inputs.h"

#include <iostream>

void Inputs::setHandler(size_t token) const
{
    const_cast<Inputs*>(this)->adoptHandler(token);
}

void Inputs::latch(bool& pulsed, Key& key)
{
    if (!key.physical)
        return;

    pulsed         = false;
    key.blocked    = true;
    key.wasDown    = false;
    key.ticksHeld  = 0;
}

void Inputs::adoptHandler(size_t token)
{
    if (token == 0 || token == activeToken)
        return;

    activeToken = token;

    latch(A, keyA);
    latch(B, keyB);
    latch(up, keyUp);
    latch(down, keyDown);
    latch(left, keyLeft);
    latch(right, keyRight);
    latch(start, keyStart);
    latch(select, keySelect);
    latch(debug, keyDebug);

    bHeld     = false;
    upHeld    = false;
    downHeld  = false;
    leftHeld  = false;
    rightHeld = false;
}

bool Inputs::consume(bool down, Key& key)
{
    key.physical = down;

    if (key.blocked)
    {
        if (!down)
        {
            key.blocked   = false;
            key.wasDown   = false;
            key.ticksHeld = 0;
        }
        return false;
    }

    if (!down)
    {
        key.wasDown   = false;
        key.ticksHeld = 0;
        return false;
    }

    if (!key.wasDown)
    {
        key.wasDown   = true;
        key.ticksHeld = 0;
        return true;
    }

    ++key.ticksHeld;
    if (key.ticksHeld < RepeatGapTicks)
        return false;

    key.ticksHeld = 0;
    return true;
}

void Inputs::update()
{
    clear();

    Uint8 const* keyState = SDL_GetKeyboardState(NULL);

    if (!keyState)
    {
        return;
    }

    bool leftDown  = keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A];
    bool rightDown = keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D];
    bool upDown    = keyState[SDL_SCANCODE_UP] || keyState[SDL_SCANCODE_W];
    bool downDown  = keyState[SDL_SCANCODE_DOWN] || keyState[SDL_SCANCODE_S];

    left  = consume(leftDown, keyLeft);
    right = consume(rightDown, keyRight);
    up    = consume(upDown, keyUp);
    down  = consume(downDown, keyDown);

    leftHeld  = leftDown && !keyLeft.blocked;
    rightHeld = rightDown && !keyRight.blocked;
    upHeld    = upDown && !keyUp.blocked;
    downHeld  = downDown && !keyDown.blocked;

    if (left)
        std::cout << "LEFT pressed" << std::endl;
    if (right)
        std::cout << "RIGHT pressed" << std::endl;
    if (up)
        std::cout << "UP pressed" << std::endl;
    if (down)
        std::cout << "DOWN pressed" << std::endl;

    bool const aDown      = consume(keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_RETURN2], keyA);
    bool const bPhysical  = keyState[SDL_SCANCODE_ESCAPE];
    bool const bDown      = consume(bPhysical, keyB);
    bool const startDown  = consume(keyState[SDL_SCANCODE_I], keyStart);
    bool const selectDown = consume(keyState[SDL_SCANCODE_K], keySelect);
    bool const debugDown  = consume(keyState[SDL_SCANCODE_P], keyDebug);

    bHeld = bPhysical && !keyB.blocked;

    if (aDown)
    {
        A = true;
        std::cout << "A pressed" << std::endl;
    }
    else if (bDown)
    {
        B = true;
        std::cout << "B pressed" << std::endl;
    }
    else if (startDown)
    {
        start = true;
        std::cout << "START pressed" << std::endl;
    }
    else if (selectDown)
    {
        select = true;
        std::cout << "SELECT pressed" << std::endl;
    }
    else if (debugDown)
    {
        debug = true;
        std::cout << "DEBUG pressed" << std::endl;
    }

    if (!((leftHeld ^ rightHeld) ^ (upHeld ^ downHeld)))
    {
        leftHeld  = false;
        rightHeld = false;
        upHeld    = false;
        downHeld  = false;
        left      = false;
        right     = false;
        up        = false;
        down      = false;
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

    bHeld     = false;
    upHeld    = false;
    downHeld  = false;
    leftHeld  = false;
    rightHeld = false;
}
