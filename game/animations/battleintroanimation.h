#ifndef BATTLEINTROANIMATION_H
#define BATTLEINTROANIMATION_H

#include "animation.h"

#include <SDL_image.h>

class BattleIntroAnimation : public Animation
{
public:
    BattleIntroAnimation(SDL_Renderer* renderer);
    ~BattleIntroAnimation();

    void setFile(std::string const& file);

    virtual bool isFinished() const;
    virtual void draw(Fps const* fps, RenderSizes rs);

private:
    float        ticksDuration = 10;
    SDL_Surface* fileSurface   = NULL;
    SDL_Texture* fileTexture   = NULL;
};

#endif // BATTLEINTROANIMATION_H
