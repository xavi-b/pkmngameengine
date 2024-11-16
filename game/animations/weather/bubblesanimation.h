#ifndef BUBBLESANIMATION_H
#define BUBBLESANIMATION_H

#include "weatheranimation.h"

#include <array>

class BubblesAnimation : public WeatherAnimation
{
public:
    BubblesAnimation(SDL_Renderer* renderer, bool night);
    ~BubblesAnimation();

    virtual std::string getStartingText() const override;
    virtual std::string getContinuingText() const override;

    virtual void start() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    static constexpr size_t NumberOfBubbles = 6;

    std::pair<SDL_Surface*, SDL_Texture*> bubbles;

    std::array<std::tuple<bool, int, int, size_t>, NumberOfBubbles> selected;

    int speed     = 4;
    int spawnTick = 12;
};

#endif // BUBBLESANIMATION_H
