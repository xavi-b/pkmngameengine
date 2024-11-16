#include "underwatergrassanimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

UnderWaterGrassAnimation::UnderWaterGrassAnimation(SDL_Renderer* renderer, bool night) : MapAnimation(renderer, night)
{
    surface = IMG_Load("resources/Graphics/Animations/underwater_bubbles.png");
    texture = RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, surface), night);

    ticksDuration = speed * nSprites;
}

UnderWaterGrassAnimation::~UnderWaterGrassAnimation()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void UnderWaterGrassAnimation::start()
{
    MapAnimation::start();

    ticks = 0;
}

void UnderWaterGrassAnimation::incrementTicks()
{
    MapAnimation::incrementTicks();

    if (int(accumulatedTicks) % speed == 0)
        ticks++;
}

void UnderWaterGrassAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    static constexpr int nw = 4;

    if (ticks >= nSprites)
        return;

    int col = (spriteStartIndex + ticks) % nw;
    int row = (spriteStartIndex + ticks) / nw;

    SDL_Rect srcRect;
    srcRect.w = MapScene::TilePixelSize;
    srcRect.h = MapScene::TilePixelSize * 2;
    srcRect.x = col * 6 * MapScene::TilePixelSize + 2.5 * MapScene::TilePixelSize;
    srcRect.y = row * 6 * MapScene::TilePixelSize + 1.5 * MapScene::TilePixelSize + 8;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}
