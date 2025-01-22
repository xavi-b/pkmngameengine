#ifndef SANDANIMATION_H
#define SANDANIMATION_H

#include "tileanimation.h"

class SandAnimation : public TileAnimation
{
public:
    SandAnimation(SDL_Renderer* renderer, bool night);

    virtual void draw(Fps const* fps, RenderSizes rs) override;

    void setBike(bool bike);

private:
    bool bike = false;
};

#endif // SANDANIMATION_H
