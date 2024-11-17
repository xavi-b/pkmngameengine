#ifndef TILEANIMATION_H
#define TILEANIMATION_H

#include "mapanimation.h"

class TileAnimation : public MapAnimation
{
public:
    TileAnimation(SDL_Renderer* renderer, std::string const& tilePath, bool night);
    ~TileAnimation();

    virtual void start() override;
    virtual void incrementTicks() override;

protected:
    SDL_Surface* surface;
    SDL_Texture* texture;

    int speed            = 1;
    int spriteStartIndex = 0;
    int nSprites         = 8;
    int ticks            = 0;
};

#endif // TILEANIMATION_H
