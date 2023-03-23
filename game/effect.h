#ifndef EFFECT_H
#define EFFECT_H

#include <vector>
#include <memory>
#include "animation.h"
#include "renderutils.h"
#include <SDL_image.h>

class Effect
{
public:
    Effect(SDL_Renderer* renderer, SDL_Texture* texture);

    virtual void apply(int tick, int duration) = 0;

protected:
    SDL_Renderer* renderer;
    SDL_Texture*  texture;
};

class RectEffect : public Effect
{
public:
    RectEffect(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect startRect, SDL_Rect endRect);

    virtual void apply(int tick, int duration) override;

protected:
    SDL_Rect startRect;
    SDL_Rect endRect;
};

class ColorEffect : public Effect
{
public:
    ColorEffect(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color startColor, SDL_Color endColor);

    virtual void apply(int tick, int duration) override;

protected:
    SDL_Color startColor;
    SDL_Color endColor;
};

#endif // EFFECT_H
