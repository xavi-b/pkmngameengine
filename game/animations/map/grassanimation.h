#ifndef GRASSANIMATION_H
#define GRASSANIMATION_H

#include "mapanimation.h"

class GrassAnimation : public MapAnimation
{
public:
    GrassAnimation(SDL_Renderer* renderer);
    ~GrassAnimation();

    virtual void start() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    SDL_Surface* surface;
    SDL_Texture* texture;

    int speed            = 2;
    int spriteStartIndex = 3;
    int nSprites         = 3;
    int ticks            = 0;
};

#endif // GRASSANIMATION_H
