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
    bool         isRunning() const;
    virtual void draw(const Fps* fps, RenderSizes rs) = 0;
    virtual void forceEnd();

protected:
    SDL_Renderer* renderer;

    bool         running = false;
    unsigned int ticks   = 0;
};

#endif // ANIMATION_H
