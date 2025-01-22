#ifndef BIKESPRITE_H
#define BIKESPRITE_H

#include "sprite.h"

class BikeSprite : public Sprite
{
public:
    BikeSprite(SDL_Renderer* renderer);

    virtual void draw(Entity const& entity, Fps const* fps, RenderSizes rs, SDL_Rect dstRect) override;
};

#endif // BIKESPRITE_H
