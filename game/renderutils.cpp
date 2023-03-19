#include "renderutils.h"

#include <iostream>

RenderUtils* RenderUtils::instance()
{
    static RenderUtils i;
    return &i;
}

void RenderUtils::drawText(SDL_Renderer* renderer, RenderSizes rs, const std::string& text, SDL_Color color, SDL_Color bgColor, int x, int y)
{
    SDL_Rect rect;

    SDL_Surface* bgSurface1 = TTF_RenderText_Solid(RenderUtils::instance()->font, text.c_str(), bgColor);
    SDL_Texture* bgTexture1 = SDL_CreateTextureFromSurface(renderer, bgSurface1);
    rect.x                  = x + 2 * rs.ww / rs.aw;
    rect.y                  = y;
    rect.w                  = bgSurface1->w * rs.ww / rs.aw;
    rect.h                  = bgSurface1->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, bgTexture1, NULL, &rect);
    SDL_DestroyTexture(bgTexture1);
    SDL_FreeSurface(bgSurface1);

    SDL_Surface* bgSurface2 = TTF_RenderText_Solid(RenderUtils::instance()->font, text.c_str(), bgColor);
    SDL_Texture* bgTexture2 = SDL_CreateTextureFromSurface(renderer, bgSurface2);
    rect.x                  = x;
    rect.y                  = y + 2 * rs.wh / rs.ah;
    rect.w                  = bgSurface2->w * rs.ww / rs.aw;
    rect.h                  = bgSurface2->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, bgTexture2, NULL, &rect);
    SDL_DestroyTexture(bgTexture2);
    SDL_FreeSurface(bgSurface2);

    SDL_Surface* surface = TTF_RenderText_Solid(RenderUtils::instance()->font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.x               = x;
    rect.y               = y;
    rect.w               = surface->w * rs.ww / rs.aw;
    rect.h               = surface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

RenderUtils::RenderUtils()
{
    font = TTF_OpenFont("resources/Fonts/power clear.ttf", FontSize);
    if (font == NULL)
        std::cerr << TTF_GetError() << std::endl;
}

RenderUtils::~RenderUtils()
{
    TTF_CloseFont(font);
}
