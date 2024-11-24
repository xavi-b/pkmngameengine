#ifndef LOCATIONANIMATION_H
#define LOCATIONANIMATION_H

#include "animation.h"

class LocationAnimation : public Animation
{
public:
    LocationAnimation(SDL_Renderer* renderer);
    ~LocationAnimation();

    virtual void reset() override;
    virtual void restart() override;
    virtual void incrementTicks() override;
    virtual bool isFinished() const override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    void restart(std::string const& location);

private:
    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;

    float ticksDuration = 16;

    std::string text;
    std::string nextText;
    bool        toRestart = false;
};

#endif // LOCATIONANIMATION_H
