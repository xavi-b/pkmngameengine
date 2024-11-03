#include "weatheranimation.h"

WeatherAnimation::WeatherAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
}

bool WeatherAnimation::isFinished() const
{
    if (accumulatedTicks > ticksDuration)
        return true;

    return Animation::isFinished();
}
