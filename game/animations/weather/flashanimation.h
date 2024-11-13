#ifndef FLASHANIMATION_H
#define FLASHANIMATION_H

#include "animation.h"

#include <SDL_image.h>

class FlashAnimation : public Animation
{
public:
    FlashAnimation(SDL_Renderer* renderer);
    ~FlashAnimation();

    virtual bool isFinished() const;
    virtual void draw(Fps const* fps, RenderSizes rs);

private:
    float        ticksDuration = 10;
    SDL_Surface* fileSurface   = NULL;
    SDL_Texture* fileTexture   = NULL;
};

#endif // FLASHANIMATION_H
