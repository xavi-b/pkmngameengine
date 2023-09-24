#ifndef SEQUENTIALANIMATION_H
#define SEQUENTIALANIMATION_H

#include "animation.h"

#include <memory>
#include <vector>

class SequentialAnimation : public Animation
{
public:
    SequentialAnimation(SDL_Renderer* renderer);

    virtual void start() override;
    virtual void stop() override;
    virtual void reset() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;
    virtual void forceEnd() override;

    void addAnimation(std::unique_ptr<Animation>&& animation);

protected:
    unsigned int currentIndex = 0;

    std::vector<std::unique_ptr<Animation>> animations;
};

#endif // SEQUENTIALANIMATION_H
