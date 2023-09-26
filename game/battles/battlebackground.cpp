#include "battlebackground.h"

#include "game.h"
#include "renderutils.h"

#include <SDL_ttf.h>
#include <iostream>

BattleBackground::BattleBackground(SDL_Renderer* renderer) : renderer(renderer)
{
    bgSurface    = IMG_Load("resources/Graphics/Battlebacks/field_bg.png");
    bgTexture    = SDL_CreateTextureFromSurface(renderer, bgSurface);
    base1Surface = IMG_Load("resources/Graphics/Battlebacks/field_base1.png");
    base1Texture = SDL_CreateTextureFromSurface(renderer, base1Surface);
}

BattleBackground::~BattleBackground()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(base1Texture);
    SDL_FreeSurface(base1Surface);
}

void BattleBackground::update(Inputs const* /*inputs*/)
{
}

void BattleBackground::draw(Fps const* /*fps*/, RenderSizes rs)
{
    SDL_Rect dstRect;

    dstRect.w = bgSurface->w * rs.ww / rs.aw;
    dstRect.h = bgSurface->h * rs.wh / rs.ah;
    dstRect.x = 0;
    dstRect.y = 0;
    SDL_RenderCopy(renderer, bgTexture, NULL, &dstRect);

    dstRect.w = base1Surface->w * rs.ww / rs.aw;
    dstRect.h = base1Surface->h * rs.wh / rs.ah;
    dstRect.x = rs.ww - dstRect.w;
    dstRect.y = (bgSurface->h * rs.wh / rs.ah - dstRect.h) / 2;
    SDL_RenderCopy(renderer, base1Texture, NULL, &dstRect);

    SDL_Rect srcRect;
    srcRect.w = base1Surface->w;
    srcRect.h = base1Surface->h / 2;
    srcRect.x = 0;
    srcRect.y = 0;
    dstRect.w = srcRect.w * rs.ww / rs.aw;
    dstRect.h = srcRect.h * rs.wh / rs.ah;
    dstRect.x = 0;
    dstRect.y = bgSurface->h * rs.wh / rs.ah - dstRect.h;
    SDL_RenderCopy(renderer, base1Texture, &srcRect, &dstRect);
}
