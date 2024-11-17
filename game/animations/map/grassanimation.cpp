#include "grassanimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

GrassAnimation::GrassAnimation(SDL_Renderer* renderer, bool night)
    : TileAnimation(renderer, "resources/Graphics/Animations/Overworld dust and grass.png", night)
{
    speed            = 2;
    spriteStartIndex = 3;
    nSprites         = 3;
}

void GrassAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    static constexpr int nw = 4;

    if (ticks >= nSprites)
        return;

    int col = (spriteStartIndex + ticks) % nw;
    int row = (spriteStartIndex + ticks) / nw;

    SDL_Rect srcRect;
    srcRect.w = MapScene::TilePixelSize;
    srcRect.h = MapScene::TilePixelSize;
    srcRect.x = col * 6 * MapScene::TilePixelSize + 2.5 * MapScene::TilePixelSize;
    srcRect.y = row * 6 * MapScene::TilePixelSize + 2.5 * MapScene::TilePixelSize;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}
