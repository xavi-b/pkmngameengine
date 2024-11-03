#include "fadeanimation.h"

FadeAnimation::FadeAnimation(SDL_Renderer* renderer, bool in) : Animation(renderer), in(in)
{
}

bool FadeAnimation::isFinished() const
{
    if (accumulatedTicks > ticksDuration)
        return true;

    return Animation::isFinished();
}

void FadeAnimation::draw(Fps const* fps, RenderSizes /*rs*/)
{
    float durationPercentage = (accumulatedTicks + fps->tickPercentage()) / ticksDuration;
    if (durationPercentage > 1.0)
        durationPercentage = 1.0;

    unsigned char threshold = (in ? (1.0 - durationPercentage) : (durationPercentage)) * 255;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, threshold);

    SDL_RenderFillRect(renderer, NULL);
}
