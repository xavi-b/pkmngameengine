#include "shakeanimation.h"

ShakeAnimation::ShakeAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
}

bool ShakeAnimation::isFinished() const
{
    if (accumulatedTicks > ticksDuration)
        return true;

    return Animation::isFinished();
}

void ShakeAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

int ShakeAnimation::getShakeX(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    return 0;
}

int ShakeAnimation::getShakeY(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    return 0;
}
