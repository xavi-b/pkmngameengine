#include "erasablesprite.h"

#include "scenes/mapscene.h"

ErasableSprite::ErasableSprite(SDL_Renderer* renderer) : SquareSprite(renderer)
{
}

void ErasableSprite::updateSpriteRow(Entity const& /*entity*/)
{
    spriteRow = 3;

    if (accumulatedTicks < 4)
        spriteRow = accumulatedTicks % 4;
}

void ErasableSprite::draw(Entity const& /*entity*/, Fps const* /*fps*/, RenderSizes rs, SDL_Rect dstRect)
{
    SDL_Rect srcRect;
    srcRect.x = 0;
    srcRect.y = spriteRow * MapScene::TilePixelSize;
    srcRect.w = MapScene::TilePixelSize;
    srcRect.h = MapScene::TilePixelSize;

    dstRect.y += dstRect.h - srcRect.h * rs.wh / rs.ah;
    dstRect.h = srcRect.h * rs.wh / rs.ah;

    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}
