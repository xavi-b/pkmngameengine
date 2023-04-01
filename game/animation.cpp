#include "animation.h"

#include <iostream>

Animation::Animation(SDL_Renderer* renderer) : renderer(renderer)
{
}

void Animation::start()
{
    started = true;
}

void Animation::stop()
{
    started  = false;
    finished = true;
}

void Animation::reset()
{
    accumulatedTicks = 0;
}

void Animation::restart()
{
    reset();
    start();
}

void Animation::incrementTicks()
{
    if (started && !finished)
        accumulatedTicks++;
}

bool Animation::isStarted() const
{
    return started;
}

bool Animation::isFinished() const
{
    return finished;
}

void Animation::forceEnd()
{
    finished = true;
}
