#include "animation.h"

Animation::Animation(SDL_Renderer* renderer) : renderer(renderer)
{
}

void Animation::start()
{
    running = true;
}

void Animation::stop()
{
    running = false;
}

void Animation::reset()
{
    ticks = 0;
}

void Animation::restart()
{
    reset();
    start();
}

void Animation::incrementTicks()
{
    ticks++;
}

bool Animation::isRunning() const
{
    return running;
}

void Animation::forceEnd()
{
    ticks   = 0;
    running = false;
}
