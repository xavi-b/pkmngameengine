#include "fallentranceanimation.h"

FallEntranceAnimation::FallEntranceAnimation(SDL_Renderer* renderer, bool night) : MapAnimation(renderer, night)
{
    ticksDuration = 4;
}

void FallEntranceAnimation::draw(Fps const* fps, RenderSizes rs)
{
    if (!isFinished())
    {
        int offset = rs.ah * 2 / 3 * (1.0 - (accumulatedTicks + fps->tickPercentage()) / ticksDuration);
        if (offset >= 0)
            dstRect.y -= offset;
    }

    sprite->updateSpriteRow(*entity);
    sprite->draw(*entity, fps, rs, dstRect);
}

void FallEntranceAnimation::setEntitySprite(Entity* entity, Sprite* sprite)
{
    this->entity = entity;
    this->sprite = sprite;
}
