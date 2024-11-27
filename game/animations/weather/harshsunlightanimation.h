#ifndef HARSHSUNLIGHTMANIMATION_H
#define HARSHSUNLIGHTMANIMATION_H

#include "weatheranimation.h"

class HarshSunlightAnimation : public WeatherAnimation
{
public:
    HarshSunlightAnimation(SDL_Renderer* renderer, bool night);
    ~HarshSunlightAnimation();

    virtual std::string getStartingText() const override;
    virtual std::string getContinuingText() const override;

    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    SDL_Surface* surface;
    SDL_Texture* texture;

    int speed         = 4;
    int flareDuration = 32;
    int flareSpawn    = 64;

    bool night = false;
};

#endif // HARSHSUNLIGHTMANIMATION_H
