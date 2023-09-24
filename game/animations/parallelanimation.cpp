#include "parallelanimation.h"

#include <iostream>

ParallelAnimation::ParallelAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
}

void ParallelAnimation::start()
{
    for (auto& a : animations)
        a->start();
    Animation::start();
}

void ParallelAnimation::reset()
{
    for (auto& a : animations)
        a->reset();
    Animation::reset();
}

void ParallelAnimation::incrementTicks()
{
    if (!isStarted() || isFinished())
        return;

    bool oneIsRunning = false;
    for (auto& a : animations)
    {
        a->incrementTicks();

        if (!a->isFinished())
        {
            oneIsRunning = true;
        }
    }

    if (!oneIsRunning)
    {
        forceEnd();
    }
}

void ParallelAnimation::draw(Fps const* fps, RenderSizes rs)
{
    for (auto& a : animations)
        a->draw(fps, rs);
}

void ParallelAnimation::forceEnd()
{
    for (auto& a : animations)
        a->forceEnd();
    Animation::forceEnd();
}

void ParallelAnimation::addAnimation(std::unique_ptr<Animation>&& animation)
{
    this->animations.emplace_back(std::move(animation));
}
