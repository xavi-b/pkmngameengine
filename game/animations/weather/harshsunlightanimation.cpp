#include "harshsunlightanimation.h"

#include "utils.h"

#include <SDL_image.h>

HarshSunlightAnimation::HarshSunlightAnimation(SDL_Renderer* renderer, bool night)
    : WeatherAnimation(renderer, night), night(night)
{
    surface = IMG_Load("resources/Graphics/Weather/flare.png");
    texture = RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, surface), false);
}

HarshSunlightAnimation::~HarshSunlightAnimation()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

std::string HarshSunlightAnimation::getStartingText() const
{
    return lc::translate("The sunlight got bright!");
}

std::string HarshSunlightAnimation::getContinuingText() const
{
    return lc::translate("The sunlight is strong.");
}

void HarshSunlightAnimation::draw(Fps const* fps, RenderSizes rs)
{
    static constexpr int NbFlares = 5;

    if (night)
        return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 248, 216, 100);
    SDL_RenderFillRect(renderer, NULL);

    for (int i = 0; i < NbFlares; ++i)
    {
        float    offset = (int(accumulatedTicks) % flareSpawn + fps->tickPercentage()) / float(speed);
        SDL_Rect dstRect;
        dstRect.x   = (i - NbFlares + offset) * 2 / surface->w * rs.ww;
        dstRect.y   = (i - NbFlares + offset) * 2 / surface->h * rs.wh;
        float scale = 0.5 + (1 + i + offset) / 8;
        dstRect.w   = scale * surface->w * rs.ww / rs.aw;
        dstRect.h   = scale * surface->h * rs.wh / rs.ah;

        int currentFlareSpawnTicks = int(accumulatedTicks) % flareSpawn;
        if (currentFlareSpawnTicks > flareDuration)
            SDL_SetTextureAlphaMod(texture, 0);
        else
            SDL_SetTextureAlphaMod(texture, 255 * (1.0 - currentFlareSpawnTicks / float(flareDuration)));

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    }
}
