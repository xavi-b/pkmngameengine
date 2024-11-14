#ifndef SQUARESPRITE_H
#define SQUARESPRITE_H

#include "sprite.h"

class SquareSprite : public Sprite
{
public:
    SquareSprite(SDL_Renderer* renderer);

    virtual void draw(Entity const& entity, Fps const* fps, RenderSizes rs, SDL_Rect dstRect) override;
};

#endif // SQUARESPRITE_H
