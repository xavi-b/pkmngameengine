#include "fallexitanimation.h"

FallExitAnimation::FallExitAnimation(SDL_Renderer* renderer, bool night) : MapAnimation(renderer, night)
{
    ticksDuration = 4;
}

void FallExitAnimation::draw(Fps const* fps, RenderSizes rs)
{
    if (accumulatedTicks < ticksDuration / 2)
    {
        sprite->updateSpriteRow(*entity);
        sprite->draw(*entity, fps, rs, dstRect);
    }
}

void FallExitAnimation::setEntitySprite(Entity* entity, Sprite* sprite)
{
    this->entity = entity;
    this->sprite = sprite;
}
