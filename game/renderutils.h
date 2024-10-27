#ifndef RENDERUTILS_H
#define RENDERUTILS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

struct RenderSizes
{
    int ww = 0;
    int wh = 0;
    int aw = 1;
    int ah = 1;
};

class RenderUtils
{
public:
    constexpr static int TextSize    = 24;
    static constexpr int TextPadding = 10;

    TTF_Font* font;

    static RenderUtils* instance();

    static void drawWhiteText(SDL_Renderer*      renderer,
                              RenderSizes        rs,
                              std::string const& text,
                              int                fontSize,
                              int                x,
                              int                y,
                              int                w = 0);
    static int  drawGreyText(SDL_Renderer*      renderer,
                             RenderSizes        rs,
                             std::string const& text,
                             int                fontSize,
                             int                x,
                             int                y,
                             int                w = 0);
    static void drawWhiteTextRightAligned(SDL_Renderer*      renderer,
                                          RenderSizes        rs,
                                          std::string const& text,
                                          int                fontSize,
                                          int                x,
                                          int                y);
    static void drawGreyTextRightAligned(SDL_Renderer*      renderer,
                                         RenderSizes        rs,
                                         std::string const& text,
                                         int                fontSize,
                                         int                x,
                                         int                y);
    static void drawWhiteTextCentered(SDL_Renderer*      renderer,
                                      RenderSizes        rs,
                                      std::string const& text,
                                      int                fontSize,
                                      SDL_Rect           textRect);
    static void drawTextWrapped(SDL_Renderer*      renderer,
                                RenderSizes        rs,
                                std::string const& text,
                                SDL_Color          color,
                                SDL_Color          bgColor,
                                int                fontSize,
                                int                x,
                                int                y,
                                int                width);
    static void drawTextWithIntroWrapped(SDL_Renderer*      renderer,
                                         RenderSizes        rs,
                                         std::string const& text,
                                         SDL_Color          color,
                                         SDL_Color          bgColor,
                                         SDL_Color          introColor,
                                         SDL_Color          introBgColor,
                                         int                fontSize,
                                         int                x,
                                         int                y,
                                         int                width);
    static void drawGreyTextWithIntroWrapped(SDL_Renderer*      renderer,
                                             RenderSizes        rs,
                                             std::string const& text,
                                             SDL_Color          introColor,
                                             SDL_Color          introBgColor,
                                             int                fontSize,
                                             int                x,
                                             int                y,
                                             int                width);
    static void drawUnicodeSymbol(SDL_Renderer*  renderer,
                                  RenderSizes    rs,
                                  wchar_t const& symbol,
                                  SDL_Color      color,
                                  SDL_Color      bgColor,
                                  int            fontSize,
                                  int            x,
                                  int            y);
    static void drawBorderImage(SDL_Renderer* renderer,
                                RenderSizes   rs,
                                SDL_Surface*  surface,
                                SDL_Texture*  texture,
                                SDL_Rect      rect,
                                int           px,
                                int           py);

private:
    RenderUtils();
    ~RenderUtils();

    static int drawText(SDL_Renderer*      renderer,
                        RenderSizes        rs,
                        std::string const& text,
                        SDL_Color          color,
                        SDL_Color          bgColor,
                        int                fontSize,
                        int                x,
                        int                y,
                        int                w = 0);
};

#endif // RENDERUTILS_H
