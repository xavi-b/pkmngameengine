#include "effect.h"

Effect::Effect(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface)
    : renderer(renderer), texture(texture)
{
}

MoveEffect::MoveEffect(
    SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface, SDL_Point startRect, SDL_Point endRect)
    : Effect(renderer, texture, surface), startRect(startRect), endRect(endRect)
{
}

void MoveEffect::apply(int ticks, int duration, SDL_Rect& dstRect)
{
    float pos = 1.0 * ticks / duration;
    dstRect.x = dstRect.x + startRect.x + (endRect.x - startRect.x) * pos;
    dstRect.y = dstRect.y + startRect.y + (endRect.y - startRect.y) * pos;
}

SizeEffect::SizeEffect(
    SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface, SDL_Point startRect, SDL_Point endRect)
    : Effect(renderer, texture, surface), startRect(startRect), endRect(endRect)
{
}

void SizeEffect::apply(int ticks, int duration, SDL_Rect& dstRect)
{
    float pos = 1.0 * ticks / duration;
    dstRect.w = startRect.x + (endRect.x - startRect.x) * pos;
    dstRect.h = startRect.y + (endRect.x - startRect.y) * pos;
}

ColorEffect::ColorEffect(
    SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface, SDL_Color startColor, SDL_Color endColor)
    : Effect(renderer, texture, surface), startColor(startColor), endColor(endColor)
{
}

void ColorEffect::apply(int ticks, int duration, SDL_Rect& /*dstRect*/)
{
    float pos = 1.0 * ticks / duration;

    SDL_Color c;
    c.r = startColor.r + (endColor.r - startColor.r) * pos;
    c.g = startColor.g + (endColor.g - startColor.g) * pos;
    c.b = startColor.b + (endColor.b - startColor.b) * pos;
    c.a = startColor.a + (endColor.a - startColor.a) * pos;
    SDL_SetTextureColorMod(texture, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(texture, c.a);
}
