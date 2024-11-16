#include "squaresprite.h"

#include "scenes/mapscene.h"

SquareSprite::SquareSprite(SDL_Renderer* renderer) : Sprite(renderer)
{
}

void SquareSprite::draw(Entity const& entity, Fps const* fps, RenderSizes rs, SDL_Rect dstRect)
{
    switch (entity.direction)
    {
    case Entity::Direction::UP:
        spriteRow = 3;
        break;
    case Entity::Direction::DOWN:
        spriteRow = 0;
        break;
    case Entity::Direction::LEFT:
        spriteRow = 1;
        break;
    case Entity::Direction::RIGHT:
        spriteRow = 2;
        break;
    default:
        break;
    }

    int imageCol = 0;
    if (entity.direction != Entity::Direction::NONE)
    {
        imageCol = std::floor((accumulatedTicks + fps->tickPercentage()) / int(entity.speed) * 4);
    }

    SDL_Rect srcRect;
    srcRect.x = imageCol * MapScene::TilePixelSize;
    srcRect.y = spriteRow * MapScene::TilePixelSize;
    srcRect.w = MapScene::TilePixelSize;
    srcRect.h = MapScene::TilePixelSize;

    dstRect.y += dstRect.h - srcRect.h * rs.wh / rs.ah;
    dstRect.h = srcRect.h * rs.wh / rs.ah;

    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}
