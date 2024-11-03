#ifndef RAINANIMATION_H
#define RAINANIMATION_H

#include "weatheranimation.h"

class RainAnimation : public WeatherAnimation
{
public:
    RainAnimation(SDL_Renderer* renderer);

    virtual std::string getStartingText() const;
    virtual std::string getContinuingText() const;

    virtual void draw(Fps const* fps, RenderSizes rs);
};

#endif // RAINANIMATION_H
