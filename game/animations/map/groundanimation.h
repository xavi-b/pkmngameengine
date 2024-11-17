#ifndef GROUNDANIMATION_H
#define GROUNDANIMATION_H

#include "tileanimation.h"

class GroundAnimation : public TileAnimation
{
public:
    GroundAnimation(SDL_Renderer* renderer, bool night);

    virtual void draw(Fps const* fps, RenderSizes rs) override;
};

#endif // GROUNDANIMATION_H
