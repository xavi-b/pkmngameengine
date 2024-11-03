#include "eyeanimation.h"

#include <iostream>

EyeAnimation::EyeAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
}

bool EyeAnimation::isFinished() const
{
    if (accumulatedTicks > ticksDuration)
        return true;

    return Animation::isFinished();
}

void EyeAnimation::draw(Fps const* fps, RenderSizes rs)
{
    SDL_Rect dstRect;

    int width  = rs.ww;
    int height = rs.wh;

    float durationPercentage = (accumulatedTicks + fps->tickPercentage()) / ticksDuration;
    if (durationPercentage > 1.0)
        durationPercentage = 1.0;

    int threshold = (1.0 - durationPercentage) * height / 2;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.w = width;
    dstRect.h = threshold;
    SDL_RenderFillRect(renderer, &dstRect);

    dstRect.x = 0;
    dstRect.y = height - threshold;
    dstRect.w = width;
    dstRect.h = threshold;
    SDL_RenderFillRect(renderer, &dstRect);
}
