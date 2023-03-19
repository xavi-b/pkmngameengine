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
    constexpr static int FontSize = 24;

    static RenderUtils* instance();

    static void drawText(SDL_Renderer* renderer, RenderSizes rs, const std::string& text, SDL_Color color, SDL_Color bgColor, int x, int y);

private:
    RenderUtils();
    ~RenderUtils();

    TTF_Font* font;
};

#endif // RENDERUTILS_H
