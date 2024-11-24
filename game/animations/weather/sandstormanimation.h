#ifndef SANDSTORMANIMATION_H
#define SANDSTORMANIMATION_H

#include "weatheranimation.h"

class SandstormAnimation : public WeatherAnimation
{
public:
    SandstormAnimation(SDL_Renderer* renderer, bool night);
    ~SandstormAnimation();

    virtual std::string getStartingText() const override;
    virtual std::string getContinuingText() const override;

    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    SDL_Surface* surface;
    SDL_Texture* texture;

    int speed = 8;
};

#endif // SANDSTORMANIMATION_H
