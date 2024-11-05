#ifndef ANIMATION_H
#define ANIMATION_H

#include "fps.h"
#include "renderutils.h"

#include <memory>
#include <vector>

class Animation
{
public:
    Animation(SDL_Renderer* renderer);

    virtual void start();
    virtual void stop();
    virtual void reset();
    void         restart();
    virtual void incrementTicks();
    virtual bool isStarted() const;
    virtual bool isFinished() const;
    virtual void draw(Fps const* fps, RenderSizes rs) = 0;
    virtual void forceEnd();

protected:
    SDL_Renderer* renderer;

    bool  started          = false;
    bool  finished         = false;
    float accumulatedTicks = 0;
};

#endif // ANIMATION_H
