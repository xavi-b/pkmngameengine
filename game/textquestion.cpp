#include "textquestion.h"

#include "renderutils.h"
#include <SDL_ttf.h>
#include <iostream>

TextQuestion::TextQuestion(SDL_Renderer* renderer) : renderer(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
}

TextQuestion::~TextQuestion()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
}

void TextQuestion::init()
{
    reset();
}

void TextQuestion::update(const Inputs* inputs)
{
    if (inputs->A)
    {
        selected = true;
    }
    else if (inputs->up)
    {
        if (texts.size() > 0)
            currentIndex = (currentIndex - 1 + texts.size()) % texts.size();
    }
    else if (inputs->down)
    {
        if (texts.size() > 0)
            currentIndex = (currentIndex + 1) % texts.size();
    }
}

void TextQuestion::draw(const Fps* /*fps*/, RenderSizes rs)
{
    int borderSize    = 14;
    int dstBorderSize = borderSize * rs.wh / rs.ah;

    int height    = 2 * bgSurface->h;
    int dstHeight = height * rs.wh / rs.ah;

    // TODO pos
    SDL_Rect rect;
    rect.x = 0;
    rect.w = rs.ww;
    rect.h = dstHeight;
    rect.y = rs.wh - rect.h;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int fontSize         = 24;
    int textBoxHeight    = 2 /* lines */ * 24;
    int dstTextBoxHeight = textBoxHeight * rs.wh / rs.ah;
    int padding          = (height - textBoxHeight) / 2;
    int dstPadding       = (dstHeight - dstTextBoxHeight) / 2;

    int i = 0;
    for (const auto& text : texts)
    {
        RenderUtils::drawGreyText(renderer,
                                  rs,
                                  text,
                                  fontSize,
                                  rect.x + dstPadding,
                                  rect.y + dstPadding * 2 - dstBorderSize * 2 + i * fontSize * rs.wh / rs.ah,
                                  rs.aw - padding * 2);
        if (i == currentIndex)
        {
            // TODO cursor
        }
        ++i;
    }
}

bool TextQuestion::isFinished() const
{
    return selected;
}

void TextQuestion::reset()
{
    currentIndex = 0;
    selected     = false;
}

void TextQuestion::setTexts(const std::vector<std::string>& texts)
{
    this->texts = texts;
}

int TextQuestion::selectedIndex() const
{
    return currentIndex;
}
