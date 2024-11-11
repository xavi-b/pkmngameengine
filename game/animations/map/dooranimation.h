#ifndef DOORANIMATION_H
#define DOORANIMATION_H

#include "mapanimation.h"

class DoorAnimation : public MapAnimation
{
public:
    DoorAnimation(SDL_Renderer* renderer);
    ~DoorAnimation();

    virtual void start() override;
    virtual void incrementTicks() override;
    virtual bool isFinished() const override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    void setSprite(std::pair<SDL_Surface*, SDL_Texture*> sprite);
    void setSourceRect(SDL_Rect const& srcRect);
    void setInverted(bool newInverted);

private:
    SDL_Surface* surface;
    SDL_Texture* texture;
    SDL_Rect     srcRect;

    int speed    = 2;
    int nSprites = 0;
    int ticks    = 0;

    bool inverted = false;
};

#endif // DOORANIMATION_H
