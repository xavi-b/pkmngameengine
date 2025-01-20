#ifndef SANDANIMATION_H
#define SANDANIMATION_H

#include "tileanimation.h"

class SandAnimation : public TileAnimation
{
public:
    SandAnimation(SDL_Renderer* renderer, bool night);

    virtual void draw(Fps const* fps, RenderSizes rs) override;
};

#endif // SANDANIMATION_H
