#ifndef EFFECT_H
#define EFFECT_H

#include "renderutils.h"

#include <SDL.h>
#include <SDL_image.h>
#include <memory>
#include <vector>

class Effect
{
public:
    Effect(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface);

    virtual void apply(int accumulatedTicks, int duration, RenderSizes rs, SDL_Rect& dstRect) = 0;

protected:
    SDL_Renderer* renderer;
    SDL_Texture*  texture;
    SDL_Surface*  surface;
};

class MoveEffect : public Effect
{
public:
    MoveEffect(SDL_Renderer* renderer,
               SDL_Texture*  texture,
               SDL_Surface*  surface,
               SDL_Point     startOffset,
               SDL_Point     endOffset);

    virtual void apply(int accumulatedTicks, int duration, RenderSizes rs, SDL_Rect& dstRect) override;

protected:
    SDL_Point startOffset;
    SDL_Point endOffset;
};

class SizeEffect : public Effect
{
public:
    SizeEffect(SDL_Renderer* renderer,
               SDL_Texture*  texture,
               SDL_Surface*  surface,
               SDL_Point     startRect,
               SDL_Point     endRect);

    virtual void apply(int accumulatedTicks, int duration, RenderSizes rs, SDL_Rect& dstRect) override;

protected:
    SDL_Point startRect;
    SDL_Point endRect;
};

class ColorEffect : public Effect
{
public:
    ColorEffect(SDL_Renderer* renderer,
                SDL_Texture*  texture,
                SDL_Surface*  surface,
                SDL_Color     startColor,
                SDL_Color     endColor);

    virtual void apply(int accumulatedTicks, int duration, RenderSizes rs, SDL_Rect& dstRect) override;

protected:
    SDL_Color startColor;
    SDL_Color endColor;
};

#endif // EFFECT_H
