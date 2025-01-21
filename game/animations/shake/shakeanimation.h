#ifndef SHAKEANIMATION_H
#define SHAKEANIMATION_H

#include "animation.h"

class ShakeAnimation : public Animation
{
public:
    ShakeAnimation(SDL_Renderer* renderer);

    virtual bool isFinished() const;
    virtual void draw(Fps const* fps, RenderSizes rs);

    virtual int getShakeX(Fps const* fps, RenderSizes rs);
    virtual int getShakeY(Fps const* fps, RenderSizes rs);

protected:
    float ticksDuration = 10;
};

#endif // SHAKEANIMATION_H
