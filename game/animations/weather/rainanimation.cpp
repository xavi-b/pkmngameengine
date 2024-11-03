#include "rainanimation.h"

RainAnimation::RainAnimation(SDL_Renderer* renderer) : WeatherAnimation(renderer)
{
}

std::string RainAnimation::getStartingText() const
{
    return "It started to rain !";
}

std::string RainAnimation::getContinuingText() const
{
    return "Rain continues to fall.";
}

void RainAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    // TODO
}
