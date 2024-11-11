#include "mapanimation.h"

MapAnimation::MapAnimation(SDL_Renderer* renderer, bool night) : Animation(renderer), night(night)
{
}

bool MapAnimation::isFinished() const
{
    if (accumulatedTicks > ticksDuration)
        return true;

    return Animation::isFinished();
}

void MapAnimation::setDestinationRect(SDL_Rect const& dstRect)
{
    this->dstRect = dstRect;
}
