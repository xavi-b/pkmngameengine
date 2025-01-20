#include "groundanimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

GroundAnimation::GroundAnimation(SDL_Renderer* renderer, bool night)
    : TileAnimation(renderer,
                    "resources/Graphics/Animations/Overworld dust and grass.png",
                    TileLayer::Type::SOLID_OVERLAY,
                    night)
{
    speed            = 2;
    spriteStartIndex = 0;
    nSprites         = 3;
}

void GroundAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
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
