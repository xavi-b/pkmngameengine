#include "renderutils.h"

#include <iostream>

RenderUtils* RenderUtils::instance()
{
    static RenderUtils i;
    return &i;
}

void RenderUtils::drawText(SDL_Renderer*      renderer,
                           RenderSizes        rs,
                           std::string const& text,
                           SDL_Color          color,
                           SDL_Color          bgColor,
                           int                fontSize,
                           int                x,
                           int                y,
                           int                w)
{
    if (text.empty())
        return;

    TTF_SetFontSize(RenderUtils::instance()->font, fontSize);

    int offsetX = 0;

    SDL_Rect rect;

    SDL_Surface* bgSurface1 = TTF_RenderText_Solid(RenderUtils::instance()->font, text.c_str(), bgColor);
    SDL_Texture* bgTexture1 = SDL_CreateTextureFromSurface(renderer, bgSurface1);
    if (w != 0)
        offsetX = (w - bgSurface1->w * rs.ww / rs.aw) / 2;
    rect.x = x + 2 * rs.ww / rs.aw + offsetX;
    rect.y = y;
    rect.w = bgSurface1->w * rs.ww / rs.aw;
    rect.h = bgSurface1->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, bgTexture1, NULL, &rect);
    SDL_DestroyTexture(bgTexture1);
    SDL_FreeSurface(bgSurface1);

    SDL_Surface* bgSurface2 = TTF_RenderText_Solid(RenderUtils::instance()->font, text.c_str(), bgColor);
    SDL_Texture* bgTexture2 = SDL_CreateTextureFromSurface(renderer, bgSurface2);
    if (w != 0)
        offsetX = (w - bgSurface2->w * rs.ww / rs.aw) / 2;
    rect.x = x + offsetX;
    rect.y = y + 2 * rs.wh / rs.ah;
    rect.w = bgSurface2->w * rs.ww / rs.aw;
    rect.h = bgSurface2->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, bgTexture2, NULL, &rect);
    SDL_DestroyTexture(bgTexture2);
    SDL_FreeSurface(bgSurface2);

    SDL_Surface* surface = TTF_RenderText_Solid(RenderUtils::instance()->font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (w != 0)
        offsetX = (w - surface->w * rs.ww / rs.aw) / 2;
    rect.x = x + offsetX;
    rect.y = y;
    rect.w = surface->w * rs.ww / rs.aw;
    rect.h = surface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void RenderUtils::drawWhiteText(SDL_Renderer*      renderer,
                                RenderSizes        rs,
                                std::string const& text,
                                int                fontSize,
                                int                x,
                                int                y,
                                int                w)
{
    SDL_Color color   = {255, 255, 255, 255};
    SDL_Color bgColor = {127, 127, 127, 255};
    drawText(renderer, rs, text, color, bgColor, fontSize, x, y, w);
}

void RenderUtils::drawGreyText(SDL_Renderer*      renderer,
                               RenderSizes        rs,
                               std::string const& text,
                               int                fontSize,
                               int                x,
                               int                y,
                               int                w)
{
    SDL_Color color   = {80, 80, 80, 255};
    SDL_Color bgColor = {180, 180, 180, 255};
    drawText(renderer, rs, text, color, bgColor, fontSize, x, y, w);
}

void RenderUtils::drawWhiteTextCentered(SDL_Renderer*      renderer,
                                        RenderSizes        rs,
                                        std::string const& text,
                                        int                fontSize,
                                        SDL_Rect           textRect)
{
    drawWhiteText(renderer, rs, text, fontSize, textRect.x, textRect.y, textRect.w);
}

void RenderUtils::drawTextWrapped(SDL_Renderer*      renderer,
                                  RenderSizes        rs,
                                  std::string const& text,
                                  SDL_Color          color,
                                  SDL_Color          bgColor,
                                  int                fontSize,
                                  int                x,
                                  int                y,
                                  int                width)
{
    if (text.empty())
        return;

    TTF_SetFontSize(RenderUtils::instance()->font, fontSize);

    SDL_Rect rect;

    SDL_Surface* bgSurface1 = TTF_RenderText_Solid_Wrapped(RenderUtils::instance()->font, text.c_str(), bgColor, width);
    SDL_Texture* bgTexture1 = SDL_CreateTextureFromSurface(renderer, bgSurface1);
    rect.x                  = x + 2 * rs.ww / rs.aw;
    rect.y                  = y;
    rect.w                  = bgSurface1->w * rs.ww / rs.aw;
    rect.h                  = bgSurface1->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, bgTexture1, NULL, &rect);
    SDL_DestroyTexture(bgTexture1);
    SDL_FreeSurface(bgSurface1);

    SDL_Surface* bgSurface2 = TTF_RenderText_Solid_Wrapped(RenderUtils::instance()->font, text.c_str(), bgColor, width);
    SDL_Texture* bgTexture2 = SDL_CreateTextureFromSurface(renderer, bgSurface2);
    rect.x                  = x;
    rect.y                  = y + 2 * rs.wh / rs.ah;
    rect.w                  = bgSurface2->w * rs.ww / rs.aw;
    rect.h                  = bgSurface2->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, bgTexture2, NULL, &rect);
    SDL_DestroyTexture(bgTexture2);
    SDL_FreeSurface(bgSurface2);

    SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(RenderUtils::instance()->font, text.c_str(), color, width);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.x               = x;
    rect.y               = y;
    rect.w               = surface->w * rs.ww / rs.aw;
    rect.h               = surface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void RenderUtils::drawTextWithIntroWrapped(SDL_Renderer*      renderer,
                                           RenderSizes        rs,
                                           std::string const& text,
                                           SDL_Color          color,
                                           SDL_Color          bgColor,
                                           SDL_Color          introColor,
                                           SDL_Color          introBgColor,
                                           int                fontSize,
                                           int                x,
                                           int                y,
                                           int                width)
{
    drawTextWrapped(renderer, rs, text, color, bgColor, fontSize, x, y, width);

    auto pos = text.find_first_of(':');

    if (pos == std::string::npos)
        return;

    std::string introText = text.substr(0, pos + 1);
    drawTextWrapped(renderer, rs, introText, introColor, introBgColor, fontSize, x, y, width);
}

void RenderUtils::drawGreyTextWithIntroWrapped(SDL_Renderer*      renderer,
                                               RenderSizes        rs,
                                               std::string const& text,
                                               SDL_Color          introColor,
                                               SDL_Color          introBgColor,
                                               int                fontSize,
                                               int                x,
                                               int                y,
                                               int                width)
{
    SDL_Color color   = {80, 80, 80, 255};
    SDL_Color bgColor = {180, 180, 180, 255};
    drawTextWithIntroWrapped(renderer, rs, text, color, bgColor, introColor, introBgColor, fontSize, x, y, width);
}

void RenderUtils::drawBorderImage(SDL_Renderer* renderer,
                                  RenderSizes   rs,
                                  SDL_Surface*  surface,
                                  SDL_Texture*  texture,
                                  SDL_Rect      rect,
                                  int           px,
                                  int           py)
{
    SDL_Rect srcRect;
    SDL_Rect dstRect;

    int srcW  = surface->w - 2 * px;
    int srcH  = surface->h - 2 * py;
    int dstPx = px * rs.ww / rs.aw;
    int dstPy = py * rs.wh / rs.ah;

    // CENTER
    srcRect.x = px;
    srcRect.y = py;
    srcRect.w = srcW;
    srcRect.h = srcH;
    dstRect.x = rect.x + dstPx - 2;
    dstRect.y = rect.y + dstPy - 2;
    dstRect.w = rect.w - 2 * dstPx + 4;
    dstRect.h = rect.h - 2 * dstPy + 4;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);

    // LEFT
    srcRect.x = 0;
    srcRect.y = py;
    srcRect.w = px;
    srcRect.h = srcH;
    dstRect.x = rect.x;
    dstRect.y = rect.y + dstPy - 1;
    dstRect.w = dstPx;
    dstRect.h = rect.h - 2 * dstPy + 1;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);

    // RIGHT
    srcRect.x = surface->w - px;
    srcRect.y = py;
    srcRect.w = px;
    srcRect.h = srcH;
    dstRect.x = rect.x + rect.w - dstPx;
    dstRect.y = rect.y + dstPy - 1;
    dstRect.w = dstPx;
    dstRect.h = rect.h - 2 * dstPy + 1;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);

    // TOP
    srcRect.x = px;
    srcRect.y = 0;
    srcRect.w = srcW;
    srcRect.h = py;
    dstRect.x = rect.x + dstPx - 1;
    dstRect.y = rect.y;
    dstRect.w = rect.w - 2 * dstPx + 1;
    dstRect.h = dstPy;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);

    // BOTTOM
    srcRect.x = px;
    srcRect.y = surface->h - py;
    srcRect.w = srcW;
    srcRect.h = py;
    dstRect.x = rect.x + dstPx - 1;
    dstRect.y = rect.y + rect.h - dstPy;
    dstRect.w = rect.w - 2 * dstPx + 1;
    dstRect.h = dstPy;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);

    // TOP LEFT
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = px;
    srcRect.h = py;
    dstRect.x = rect.x;
    dstRect.y = rect.y;
    dstRect.w = dstPx;
    dstRect.h = dstPy;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);

    // TOP RIGHT
    srcRect.x = surface->w - px;
    srcRect.y = 0;
    srcRect.w = px;
    srcRect.h = py;
    dstRect.x = rect.x + rect.w - dstPx;
    dstRect.y = rect.y;
    dstRect.w = dstPx;
    dstRect.h = dstPy;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);

    // BOTTOM LEFT
    srcRect.x = 0;
    srcRect.y = surface->h - py;
    srcRect.w = px;
    srcRect.h = py;
    dstRect.x = rect.x;
    dstRect.y = rect.y + rect.h - dstPy;
    dstRect.w = dstPx;
    dstRect.h = dstPy;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);

    // BOTTOM RIGHT
    srcRect.x = surface->w - px;
    srcRect.y = surface->h - py;
    srcRect.w = px;
    srcRect.h = py;
    dstRect.x = rect.x + rect.w - dstPx;
    dstRect.y = rect.y + rect.h - dstPy;
    dstRect.w = dstPx;
    dstRect.h = dstPy;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}

RenderUtils::RenderUtils()
{
    font = TTF_OpenFont("resources/Fonts/power green.ttf", TextSize);
    if (font == NULL)
        std::cerr << TTF_GetError() << std::endl;
}

RenderUtils::~RenderUtils()
{
    TTF_CloseFont(font);
}
