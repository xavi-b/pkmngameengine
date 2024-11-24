#include "blizzardanimation.h"

#include "utils.h"

#include <SDL_image.h>

BlizzardAnimation::BlizzardAnimation(SDL_Renderer* renderer, bool night) : WeatherAnimation(renderer, night)
{
    surface = IMG_Load("resources/Graphics/Weather/blizzard_tile.png");
    texture = RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, surface), night);
}

BlizzardAnimation::~BlizzardAnimation()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

std::string BlizzardAnimation::getStartingText() const
{
    return lc::translate("A blizzard is raging !");
}

std::string BlizzardAnimation::getContinuingText() const
{
    return lc::translate("The blizzard rages.");
}

void BlizzardAnimation::draw(Fps const* fps, RenderSizes rs)
{
    size_t nw = rs.aw / surface->w + 2;
    size_t nh = rs.ah / surface->h + 3;

    for (size_t i = 0; i < nw; ++i)
    {
        for (size_t j = 0; j < nh; ++j)
        {
            SDL_Rect dstRect;
            dstRect.w = surface->w * rs.ww / rs.aw;
            dstRect.h = surface->h * rs.wh / rs.ah;
            dstRect.x = (i - (int(accumulatedTicks) % speed + fps->tickPercentage()) / speed) * dstRect.w + offsetX;
            dstRect.y =
                (j + (int(accumulatedTicks) % (speed * 2) + fps->tickPercentage()) / (speed * 2) - 1) * dstRect.h
                + offsetY;
            SDL_RenderCopy(renderer, texture, NULL, &dstRect);
        }
    }
}
