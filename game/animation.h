#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <memory>
#include "fps.h"
#include "renderutils.h"

struct AnimationFrame
{
    int ticks    = 0;
    int duration = -1;

    virtual void draw(const Fps* fps, RenderSizes rs) = 0;
};

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
    virtual void draw(const Fps* fps, RenderSizes rs) = 0;
    virtual void forceEnd();

protected:
    SDL_Renderer* renderer;

    bool  started         = false;
    bool  finished        = false;
    float ticksPercentage = 0;
};

#endif // ANIMATION_H
