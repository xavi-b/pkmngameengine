#ifndef PARALLELANIMATION_H
#define PARALLELANIMATION_H

#include "animation.h"

#include <memory>
#include <vector>

class ParallelAnimation : public Animation
{
public:
    ParallelAnimation(SDL_Renderer* renderer);

    virtual void start() override;
    virtual void reset() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;
    virtual void forceEnd() override;

    void addAnimation(std::unique_ptr<Animation>&& animation);

protected:
    std::vector<std::unique_ptr<Animation>> animations;
};

#endif // PARALLELANIMATION_H
