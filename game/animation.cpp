#include "animation.h"

Animation::Animation()
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
    currentFrame = 0;
}

void Animation::restart()
{
    reset();
    start();
}

void Animation::incrementTicks()
{
    if (currentFrame < frames.size())
    {
        frames[currentFrame]->ticks++;
        if (frames[currentFrame]->duration >= 0 && frames[currentFrame]->ticks >= frames[currentFrame]->duration)
        {
            frames[currentFrame]->ticks = 0;
            currentFrame++;
            if (currentFrame >= frames.size())
            {
                stop();
            }
        }
    }
}

bool Animation::isRunning() const
{
    return running;
}

void Animation::draw(const Fps* /*fps*/, RenderSizes /*rs*/)
{
}

void Animation::forceEnd()
{
    currentFrame = frames.size() - 1;
    if (currentFrame < 0)
        currentFrame = 0;
    running = false;
}
