#include "weatheranimation.h"

WeatherAnimation::WeatherAnimation(SDL_Renderer* renderer, bool night) : Animation(renderer), night(night)
{
}

bool WeatherAnimation::isFinished() const
{
    if (accumulatedTicks > ticksDuration)
        return true;

    return Animation::isFinished();
}
