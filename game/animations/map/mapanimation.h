#ifndef MAPANIMATION_H
#define MAPANIMATION_H

#include "animation.h"
#include "entity.h"

class MapAnimation : public Animation
{
public:
    MapAnimation(SDL_Renderer* renderer, bool night);

    virtual bool isFinished() const;

    void setDestinationRect(SDL_Rect const& dstRect);

protected:
    float    ticksDuration = 32;
    bool     night         = false;
    SDL_Rect dstRect;
};

#endif // MAPANIMATION_H
