#ifndef INTROANIMATION_H
#define INTROANIMATION_H

#include "sequentialanimation.h"

class IntroAnimation : public SequentialAnimation
{
public:
    IntroAnimation(SDL_Renderer* renderer);
};

#endif // INTROANIMATION_H
