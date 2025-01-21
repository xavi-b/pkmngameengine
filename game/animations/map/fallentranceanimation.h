#ifndef FALLENTRANCEANIMATION_H
#define FALLENTRANCEANIMATION_H

#include "entity.h"
#include "mapanimation.h"
#include "sprite.h"

class FallEntranceAnimation : public MapAnimation
{
public:
    FallEntranceAnimation(SDL_Renderer* renderer, bool night);

    virtual void draw(Fps const* fps, RenderSizes rs) override;

    void setEntitySprite(Entity* entity, Sprite* sprite);

private:
    Entity* entity = nullptr;
    Sprite* sprite = nullptr;
};

#endif // FALLENTRANCEANIMATION_H
