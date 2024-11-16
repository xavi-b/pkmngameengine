#include "surfsprite.h"

SurfSprite::SurfSprite(SDL_Renderer* renderer) : Sprite(renderer)
{
}

void SurfSprite::draw(Entity const& entity, Fps const* fps, RenderSizes rs, SDL_Rect dstRect)
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
    srcRect.x = imageCol * surface->w / 4;
    srcRect.y = -4 + spriteRow * surface->h / 4;
    srcRect.w = surface->w / 4;
    srcRect.h = surface->h / 4;

    dstRect.x -= (srcRect.w * rs.ww / rs.aw - dstRect.w) / 2;
    dstRect.y -= (srcRect.h * rs.wh / rs.ah - dstRect.h) / 2;
    dstRect.w = srcRect.w * rs.ww / rs.aw;
    dstRect.h = srcRect.h * rs.wh / rs.ah;

    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}
