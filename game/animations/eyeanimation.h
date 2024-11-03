#ifndef EYEANIMATION_H
#define EYEANIMATION_H

#include "animation.h"

class EyeAnimation : public Animation
{
public:
    EyeAnimation(SDL_Renderer* renderer);

    virtual bool isFinished() const;
    virtual void draw(Fps const* fps, RenderSizes rs);

private:
    float ticksDuration = 8;
};

#endif // EYEANIMATION_H
