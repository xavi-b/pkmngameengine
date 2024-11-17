#ifndef GRASSANIMATION_H
#define GRASSANIMATION_H

#include "tileanimation.h"

class GrassAnimation : public TileAnimation
{
public:
    GrassAnimation(SDL_Renderer* renderer, bool night);

    virtual void draw(Fps const* fps, RenderSizes rs) override;
};

#endif // GRASSANIMATION_H
