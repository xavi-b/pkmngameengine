#ifndef MAPANIMATION_H
#define MAPANIMATION_H

#include "animation.h"

class MapAnimation : public Animation
{
public:
    MapAnimation(SDL_Renderer* renderer);

    virtual bool isFinished() const;

    void setDestinationRect(SDL_Rect const& dstRect);

protected:
    float    ticksDuration = 32;
    SDL_Rect dstRect;
};

#endif // MAPANIMATION_H
