#ifndef UNDERWATERGRASSANIMATION_H
#define UNDERWATERGRASSANIMATION_H

#include "mapanimation.h"

class UnderWaterGrassAnimation : public MapAnimation
{
public:
    UnderWaterGrassAnimation(SDL_Renderer* renderer, bool night);
    ~UnderWaterGrassAnimation();

    virtual void start() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    SDL_Surface* surface;
    SDL_Texture* texture;

    int speed            = 1;
    int spriteStartIndex = 0;
    int nSprites         = 8;
    int ticks            = 0;
};

#endif // UNDERWATERGRASSANIMATION_H
