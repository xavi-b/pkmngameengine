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
    Animation();

    void start();
    void stop();
    void reset();
    void restart();
    void incrementTicks();
    bool isRunning() const;
    void draw(const Fps* fps, RenderSizes rs);

private:
    bool         running      = false;
    unsigned int currentFrame = 0;

    std::vector<std::unique_ptr<AnimationFrame>> frames;
};

#endif // ANIMATION_H
