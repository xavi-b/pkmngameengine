#ifndef UNDERWATERGRASSANIMATION_H
#define UNDERWATERGRASSANIMATION_H

#include "tileanimation.h"

class UnderWaterGrassAnimation : public TileAnimation
{
public:
    UnderWaterGrassAnimation(SDL_Renderer* renderer, bool night);

    virtual void draw(Fps const* fps, RenderSizes rs) override;
};

#endif // UNDERWATERGRASSANIMATION_H
