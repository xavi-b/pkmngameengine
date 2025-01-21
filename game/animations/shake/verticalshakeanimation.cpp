#include "verticalshakeanimation.h"

#include <cmath>
#include <numbers>

VerticalShakeAnimation::VerticalShakeAnimation(SDL_Renderer* renderer) : ShakeAnimation(renderer)
{
}

int VerticalShakeAnimation::getShakeY(Fps const* fps, RenderSizes /*rs*/)
{
    if (!isRunning())
        return 0;

    float ratio = 1.0 - (accumulatedTicks + fps->tickPercentage()) / ticksDuration;
    float sinus = std::sin(std::numbers::pi / 2 * (accumulatedTicks + fps->tickPercentage()) * 4);

    return sinus * ratio * 4;
}
