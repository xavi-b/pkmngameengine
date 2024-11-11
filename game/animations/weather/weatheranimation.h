#ifndef WEATHERANIMATION_H
#define WEATHERANIMATION_H

#include "animation.h"

class WeatherAnimation : public Animation
{
public:
    WeatherAnimation(SDL_Renderer* renderer, bool night);

    virtual bool isFinished() const;

    virtual std::string getStartingText() const   = 0;
    virtual std::string getContinuingText() const = 0;

protected:
    float ticksDuration = 32;
    bool  night         = false;
};

#endif // WEATHERANIMATION_H
