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

    void setOffsetX(int newOffsetX);
    void setOffsetY(int newOffsetY);

protected:
    float ticksDuration = 32;
    bool  night         = false;

    int offsetX = 0;
    int offsetY = 0;
};

#endif // WEATHERANIMATION_H
