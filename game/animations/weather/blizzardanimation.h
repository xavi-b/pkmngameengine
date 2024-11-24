#ifndef BLIZZARDANIMATION_H
#define BLIZZARDANIMATION_H

#include "weatheranimation.h"

class BlizzardAnimation : public WeatherAnimation
{
public:
    BlizzardAnimation(SDL_Renderer* renderer, bool night);
    ~BlizzardAnimation();

    virtual std::string getStartingText() const override;
    virtual std::string getContinuingText() const override;

    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    SDL_Surface* surface;
    SDL_Texture* texture;

    int offsetX = 0;
    int offsetY = 0;

    int speed = 16;
};

#endif // BLIZZARDANIMATION_H
