#ifndef VERTICALSHAKEANIMATION_H
#define VERTICALSHAKEANIMATION_H

#include "shakeanimation.h"

class VerticalShakeAnimation : public ShakeAnimation
{
public:
    VerticalShakeAnimation(SDL_Renderer* renderer);

    virtual int getShakeY(Fps const* fps, RenderSizes rs) override;
};

#endif // VERTICALSHAKEANIMATION_H
