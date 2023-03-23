#include "effect.h"

Effect::Effect(SDL_Renderer* renderer, SDL_Texture* texture) : renderer(renderer), texture(texture)
{
}

RectEffect::RectEffect(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect startRect, SDL_Rect endRect)
    : Effect(renderer, texture), startRect(startRect), endRect(endRect)
{
}

void RectEffect::apply(int tick, int duration)
{
    //    int dstWidth  = baseSurface->w * rs.ww / rs.aw;
    //    int dstHeight = baseSurface->h * rs.wh / rs.ah;

    //    SDL_Rect dstRect;
    //    dstRect.x = (rs.ww - dstWidth) / 2;
    //    dstRect.y = (rs.wh - dstHeight) / 2;
    //    dstRect.w = dstWidth;
    //    dstRect.h = dstHeight;
    //    SDL_RenderCopy(renderer, baseTexture, NULL, &dstRect);
}

ColorEffect::ColorEffect(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color startColor, SDL_Color endColor)
    : Effect(renderer, texture), startColor(startColor), endColor(endColor)
{
}

void ColorEffect::apply(int tick, int duration)
{
}
