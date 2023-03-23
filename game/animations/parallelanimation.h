#ifndef PARALLELANIMATION_H
#define PARALLELANIMATION_H

#include <vector>
#include <memory>
#include "animation.h"

class ParallelAnimation : public Animation
{
public:
    ParallelAnimation(SDL_Renderer* renderer);

    virtual void reset() override;
    virtual void incrementTicks() override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;
    virtual void forceEnd() override;

    void setAnimations(std::vector<std::unique_ptr<Animation>>&& animations);

protected:
    std::vector<std::unique_ptr<Animation>> animations;
};

#endif // PARALLELANIMATION_H
