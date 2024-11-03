#ifndef WEATHERANIMATION_H
#define WEATHERANIMATION_H

#include "animation.h"

class WeatherAnimation : public Animation
{
public:
    WeatherAnimation(SDL_Renderer* renderer);

    virtual bool isFinished() const;

    virtual std::string getStartingText() const   = 0;
    virtual std::string getContinuingText() const = 0;

private:
    float ticksDuration = 8;
};

#endif // WEATHERANIMATION_H
