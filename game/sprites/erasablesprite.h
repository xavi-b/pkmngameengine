#ifndef ERASABLESPRITE_H
#define ERASABLESPRITE_H

#include "squaresprite.h"

class ErasableSprite : public SquareSprite
{
public:
    ErasableSprite(SDL_Renderer* renderer);

    virtual void updateSpriteRow(Entity const& entity) override;
    virtual void draw(Entity const& entity, Fps const* fps, RenderSizes rs, SDL_Rect dstRect) override;
};

#endif // ERASABLESPRITE_H
