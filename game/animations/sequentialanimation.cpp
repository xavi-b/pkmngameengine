#include "sequentialanimation.h"

SequentialAnimation::SequentialAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
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
    if (currentIndex < animations.size())
    {
        animations[currentIndex]->incrementTicks();
        if (animations[currentIndex]->isFinished())
        {
            // frames[currentFrame]->reset();
            currentIndex++;
            if (currentIndex >= animations.size())
            {
                stop();
            }
        }
    }
}

void SequentialAnimation::draw(const Fps* fps, RenderSizes rs)
{
    if (currentIndex < animations.size())
        animations[currentIndex]->draw(fps, rs);
}

void SequentialAnimation::forceEnd()
{
    if (animations.size() > 0)
        currentIndex = animations.size() - 1;
    else
        currentIndex = 0;
    running = false;
}

void SequentialAnimation::setAnimations(std::vector<std::unique_ptr<Animation>>&& animations)
{
    this->animations = std::move(animations);
}
