#ifndef RAINANIMATION_H
#define RAINANIMATION_H

#include "weatheranimation.h"

#include <array>
#include <map>

class RainAnimation : public WeatherAnimation
{
public:
    RainAnimation(SDL_Renderer* renderer, bool night);
    ~RainAnimation();

    virtual std::string getStartingText() const override;
    virtual std::string getContinuingText() const override;

    virtual void start() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    static constexpr size_t NumberOfRains = 32;

    std::map<SDL_Surface*, SDL_Texture*> rains;

    std::array<std::tuple<bool, int, int, size_t, SDL_Surface*, SDL_Texture*>, NumberOfRains> selected;

    int speed     = 2;
    int spawnTick = 4;
};

#endif // RAINANIMATION_H
