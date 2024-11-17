#include "underwatergrassanimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

UnderWaterGrassAnimation::UnderWaterGrassAnimation(SDL_Renderer* renderer, bool night)
    : TileAnimation(renderer, "resources/Graphics/Animations/underwater_bubbles.png", night)
{
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
