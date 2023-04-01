#include "effect.h"

#include <iostream>

Effect::Effect(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface)
    : renderer(renderer), texture(texture), surface(surface)
{
}

MoveEffect::MoveEffect(
    SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface, SDL_Point startOffset, SDL_Point endOffset)
    : Effect(renderer, texture, surface), startOffset(startOffset), endOffset(endOffset)
{
}

void MoveEffect::apply(int accumulatedTicks, int duration, RenderSizes rs, SDL_Rect& dstRect)
{
    float pos = 1.0 * accumulatedTicks / duration;
    dstRect.x = dstRect.x + (startOffset.x + (endOffset.x - startOffset.x) * pos) * rs.ww / rs.aw;
    dstRect.y = dstRect.y + (startOffset.y + (endOffset.y - startOffset.y) * pos) * rs.wh / rs.ah;
}

SizeEffect::SizeEffect(
    SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface, SDL_Point startRect, SDL_Point endRect)
    : Effect(renderer, texture, surface), startRect(startRect), endRect(endRect)
{
}

void SizeEffect::apply(int accumulatedTicks, int duration, RenderSizes rs, SDL_Rect& dstRect)
{
    float pos = 1.0 * accumulatedTicks / duration;
    dstRect.w = (startRect.x + (endRect.x - startRect.x) * pos) * rs.ww / rs.aw;
    dstRect.h = (startRect.y + (endRect.x - startRect.y) * pos) * rs.wh / rs.ah;
}

ColorEffect::ColorEffect(
    SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* surface, SDL_Color startColor, SDL_Color endColor)
    : Effect(renderer, texture, surface), startColor(startColor), endColor(endColor)
{
}

void ColorEffect::apply(int accumulatedTicks, int duration, RenderSizes /*rs*/, SDL_Rect& /*dstRect*/)
{
    float pos = 1.0 * accumulatedTicks / duration;

    SDL_Color c;
    c.r = startColor.r + (endColor.r - startColor.r) * pos;
    c.g = startColor.g + (endColor.g - startColor.g) * pos;
    c.b = startColor.b + (endColor.b - startColor.b) * pos;
    c.a = startColor.a + (endColor.a - startColor.a) * pos;
    SDL_SetTextureColorMod(texture, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(texture, c.a);
}
