#include "parallelanimation.h"

ParallelAnimation::ParallelAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
}

void ParallelAnimation::reset()
{
    for (auto& a : animations)
        a->reset();
}

void ParallelAnimation::incrementTicks()
{
    for (auto& a : animations)
        a->incrementTicks();
}

void ParallelAnimation::draw(const Fps* fps, RenderSizes rs)
{
    for (auto& a : animations)
        a->draw(fps, rs);
}

void ParallelAnimation::forceEnd()
{
    for (auto& a : animations)
        a->forceEnd();
    running = false;
}

void ParallelAnimation::addAnimation(std::unique_ptr<Animation>&& animation)
{
    this->animations.emplace_back(std::move(animation));
}
