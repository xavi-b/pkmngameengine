#include "foganimation.h"

#include "utils.h"

#include <SDL_image.h>

FogAnimation::FogAnimation(SDL_Renderer* renderer, bool night) : WeatherAnimation(renderer, night)
{
    SDL_Surface* fog1Surface = IMG_Load("resources/Graphics/Weather/fog_tile.png");
    SDL_Texture* fog1Texture =
        RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, fog1Surface), night);
    fog1                     = {fog1Surface, fog1Texture};
    SDL_Surface* fog2Surface = IMG_Load("resources/Graphics/Weather/fog_tile_2.png");
    SDL_Texture* fog2Texture =
        RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, fog2Surface), night);
    fog2 = {fog2Surface, fog2Texture};
}

FogAnimation::~FogAnimation()
{
    SDL_DestroyTexture(fog1.second);
    SDL_FreeSurface(fog1.first);
    SDL_DestroyTexture(fog2.second);
    SDL_FreeSurface(fog2.first);
}

std::string FogAnimation::getStartingText() const
{
    return {};
}

std::string FogAnimation::getContinuingText() const
{
    return {};
}

void FogAnimation::start()
{
    WeatherAnimation::start();
}

void FogAnimation::incrementTicks()
{
    WeatherAnimation::incrementTicks();
}

void FogAnimation::draw(Fps const* fps, RenderSizes rs)
{
    for (int x = -TilePixelSize; x < rs.ww + TilePixelSize; x += TilePixelSize)
    {
        for (int y = 0; y < rs.wh + TilePixelSize; y += TilePixelSize)
        {
            SDL_Rect dstRect;
            dstRect.y = y * rs.wh / rs.ah;
            dstRect.w = fog1.first->w * rs.ww / rs.aw;
            dstRect.h = fog1.first->h * rs.wh / rs.ah;

            dstRect.x =
                (x + ((int(accumulatedTicks) % speed) + fps->tickPercentage()) * fog1.first->w / speed) * rs.ww / rs.aw;
            SDL_SetTextureAlphaMod(fog1.second, 127);
            SDL_RenderCopy(renderer, fog1.second, NULL, &dstRect);

            dstRect.x =
                (x - ((int(accumulatedTicks) % speed) + fps->tickPercentage()) * fog2.first->w / speed) * rs.ww / rs.aw;
            SDL_SetTextureAlphaMod(fog2.second, 127);
            SDL_RenderCopy(renderer, fog2.second, NULL, &dstRect);
        }
    }
}
