#ifndef SURFSPRITE_H
#define SURFSPRITE_H

#include "sprite.h"

class SurfSprite : public Sprite
{
public:
    SurfSprite(SDL_Renderer* renderer);

    virtual void draw(Entity const& entity, Fps const* fps, RenderSizes rs, SDL_Rect dstRect) override;
};

#endif // SURFSPRITE_H
