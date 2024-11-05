#ifndef FOGANIMATION_H
#define FOGANIMATION_H

#include "weatheranimation.h"

class FogAnimation : public WeatherAnimation
{
public:
    static constexpr int TilePixelSize = 128;

    FogAnimation(SDL_Renderer* renderer);
    ~FogAnimation();

    virtual std::string getStartingText() const override;
    virtual std::string getContinuingText() const override;

    virtual void start() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::pair<SDL_Surface*, SDL_Texture*> fog1;
    std::pair<SDL_Surface*, SDL_Texture*> fog2;

    int speed = 16;
};

#endif // FOGANIMATION_H
