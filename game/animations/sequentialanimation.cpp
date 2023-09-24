#include "sequentialanimation.h"

#include <iostream>

SequentialAnimation::SequentialAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
}

void SequentialAnimation::start()
{
    if (currentIndex < animations.size())
        animations[currentIndex]->start();
    Animation::start();
}

void SequentialAnimation::stop()
{
    if (currentIndex < animations.size())
        animations[currentIndex]->stop();
    Animation::stop();
}

void SequentialAnimation::reset()
{
    currentIndex = 0;
    if (currentIndex < animations.size())
        animations[currentIndex]->reset();
    Animation::reset();
}

void SequentialAnimation::incrementTicks()
{
    if (!isStarted() || isFinished())
        return;

    if (currentIndex < animations.size())
    {
        animations[currentIndex]->incrementTicks();
        if (animations[currentIndex]->isFinished())
        {
            // frames[currentIndex]->reset();
            if (currentIndex + 1 >= animations.size())
            {
                forceEnd();
            }
            else
            {
                currentIndex++;
                animations[currentIndex]->start();
            }
        }
    }
}

void SequentialAnimation::draw(Fps const* fps, RenderSizes rs)
{
    if (currentIndex < animations.size())
        animations[currentIndex]->draw(fps, rs);
}

void SequentialAnimation::forceEnd()
{
    Animation::forceEnd();
}

void SequentialAnimation::addAnimation(std::unique_ptr<Animation>&& animation)
{
    this->animations.emplace_back(std::move(animation));
}
