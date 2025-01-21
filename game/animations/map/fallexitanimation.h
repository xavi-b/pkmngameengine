#ifndef FALLEXITANIMATION_H
#define FALLEXITANIMATION_H

#include "entity.h"
#include "mapanimation.h"
#include "sprite.h"

class FallExitAnimation : public MapAnimation
{
public:
    FallExitAnimation(SDL_Renderer* renderer, bool night);

    virtual void draw(Fps const* fps, RenderSizes rs) override;

    void setEntitySprite(Entity* entity, Sprite* sprite);

private:
    Entity* entity = nullptr;
    Sprite* sprite = nullptr;
};

#endif // FALLEXITANIMATION_H
