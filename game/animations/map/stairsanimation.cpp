#include "stairsanimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

StairsAnimation::StairsAnimation(SDL_Renderer* renderer, Direction direction, bool night)
    : MapAnimation(renderer, night), direction(direction)
{
}

bool StairsAnimation::isFinished() const
{
    if (!entity)
        return false;

    return entity->x == entity->previousX;
}

void StairsAnimation::draw(Fps const* fps, RenderSizes rs)
{
    int dstPixelSizeHeight = MapScene::TilePixelSize * rs.wh / rs.ah;

    float delay = 1;
    if (accumulatedTicks + fps->tickPercentage() > delay)
    {
        float delta = accumulatedTicks - delay;
        if (!isFinished())
            delta += fps->tickPercentage();
        else
            delta += 1;

        switch (direction)
        {
        case ToUpstairs:
            dstRect.y -= delta * dstPixelSizeHeight / 4;
            break;
        case ToDownstairs:
            dstRect.y += delta * dstPixelSizeHeight / 4;
            break;
        case FromUpstairs:
            dstRect.y += (2.0 - delta) * dstPixelSizeHeight / 4;
            break;
        case FromDownstairs:
            dstRect.y -= (2.0 - delta) * dstPixelSizeHeight / 4;
            break;
        }
    }

    sprite->updateSpriteRow(*entity);
    sprite->draw(*entity, fps, rs, dstRect);
}

void StairsAnimation::setEntitySprite(Entity* entity, Sprite* sprite)
{
    this->entity = entity;
    this->sprite = sprite;
}
