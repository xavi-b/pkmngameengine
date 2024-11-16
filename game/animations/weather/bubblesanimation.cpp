#include "bubblesanimation.h"

#include "scenes/mapscene.h"
#include "utils.h"

#include <SDL_image.h>

BubblesAnimation::BubblesAnimation(SDL_Renderer* renderer, bool night) : WeatherAnimation(renderer, night)
{
    SDL_Surface* surface = IMG_Load("resources/Graphics/Animations/underwater_bubbles.png");
    SDL_Texture* texture = RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, surface), night);
    bubbles              = {surface, texture};
}

BubblesAnimation::~BubblesAnimation()
{
    SDL_DestroyTexture(bubbles.second);
    SDL_FreeSurface(bubbles.first);
}

std::string BubblesAnimation::getStartingText() const
{
    return lc::translate("It started to rain !");
}

std::string BubblesAnimation::getContinuingText() const
{
    return lc::translate("Rain continues to fall.");
}

void BubblesAnimation::start()
{
    WeatherAnimation::start();

    for (size_t i = 0; i < NumberOfBubbles / 2; ++i)
    {
        selected[i] = {true, 0, 0, 0};
    }
}

void BubblesAnimation::incrementTicks()
{
    WeatherAnimation::incrementTicks();

    if (int(accumulatedTicks) % spawnTick == 0)
    {
        size_t j = Utils::randuint(0, NumberOfBubbles - 1);

        selected[j] = {true, 0, 0, 0};
    }

    for (size_t i = 0; i < selected.size(); ++i)
    {
        auto& [isNew, baseX, baseY, ticks] = selected[i];

        if (isNew)
            continue;

        ticks++;
    }
}

void BubblesAnimation::draw(Fps const* fps, RenderSizes rs)
{
    for (size_t i = 0; i < selected.size(); ++i)
    {
        auto& [isNew, baseX, baseY, ticks] = selected[i];

        if (isNew)
        {
            baseX = Utils::randint(-MapScene::TilePixelSize, rs.aw + MapScene::TilePixelSize);
            baseY = Utils::randint(MapScene::TilePixelSize, rs.ah + MapScene::TilePixelSize);
        }

        isNew = false;

        SDL_Rect srcRect;
        srcRect.w = MapScene::TilePixelSize;
        srcRect.h = MapScene::TilePixelSize * 2;
        srcRect.x = 1 * 6 * MapScene::TilePixelSize + 2.5 * MapScene::TilePixelSize;
        srcRect.y = 0 * 6 * MapScene::TilePixelSize + 1.5 * MapScene::TilePixelSize + 8;

        SDL_Rect dstRect;
        dstRect.w = MapScene::TilePixelSize * rs.ww / rs.aw;
        dstRect.h = 2 * MapScene::TilePixelSize * rs.wh / rs.ah;
        dstRect.x = baseX * rs.ww / rs.aw;
        dstRect.y = (baseY - (ticks + fps->tickPercentage()) * MapScene::TilePixelSize / speed) * rs.wh / rs.ah;
        SDL_RenderCopy(renderer, bubbles.second, &srcRect, &dstRect);
    }
}
