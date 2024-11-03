#ifndef FADEANIMATION_H
#define FADEANIMATION_H

#include "animation.h"

class FadeAnimation : public Animation
{
public:
    FadeAnimation(SDL_Renderer* renderer, bool in = true);

    virtual bool isFinished() const;
    virtual void draw(Fps const* fps, RenderSizes rs);

private:
    bool  in            = true;
    float ticksDuration = 5;
};

#endif // FADEANIMATION_H
