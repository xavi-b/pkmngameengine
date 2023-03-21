#include "textquestion.h"

#include "renderutils.h"
#include "textspeech.h"
#include <SDL_ttf.h>
#include <iostream>

TextQuestion::TextQuestion(SDL_Renderer* renderer) : renderer(renderer)
{
    bgSurface        = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture        = SDL_CreateTextureFromSurface(renderer, bgSurface);
    selectionSurface = IMG_Load("resources/Graphics/Pictures/selarrow.png");
    selectionTexture = SDL_CreateTextureFromSurface(renderer, selectionSurface);
}

TextQuestion::~TextQuestion()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(selectionTexture);
    SDL_FreeSurface(selectionSurface);
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

void TextQuestion::draw(const Fps* /*fps*/, RenderSizes rs, Position pos)
{
    int borderSize    = 14;
    int dstBorderSize = borderSize * rs.wh / rs.ah;

    int fontSize  = RenderUtils::TextSize;
    int height    = texts.size() * TextSpeech::TextBoxSize;
    int dstHeight = height * rs.wh / rs.ah;

    int maxW = 0;
    for (const auto& text : texts)
    {
        int w, h;
        TTF_SetFontSize(RenderUtils::instance()->font, fontSize);
        TTF_SizeText(RenderUtils::instance()->font, text.c_str(), &w, &h);
        if (w > maxW)
            maxW = w;
    }

    int textBoxHeight    = texts.size() /* lines */ * RenderUtils::TextSize;
    int dstTextBoxHeight = textBoxHeight * rs.wh / rs.ah;
    int dstPaddingX      = (height - textBoxHeight) / 2 * rs.ww / rs.aw;
    int dstPaddingY      = (dstHeight - dstTextBoxHeight) / 2;

    SDL_Rect rect;
    rect.w = (2 * selectionSurface->w + maxW + dstPaddingX * 2) * rs.ww / rs.aw;
    rect.x = pos == Left ? 0 : rs.ww - rect.w;
    rect.h = dstHeight;
    rect.y = rs.wh - (2 * (TextSpeech::TextBoxSize * rs.wh / rs.ah)) - rect.h;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int i = 0;
    for (const auto& text : texts)
    {
        RenderUtils::drawGreyText(renderer,
                                  rs,
                                  text,
                                  fontSize,
                                  rect.x + dstPaddingX + 2 * selectionSurface->w * rs.ww / rs.aw,
                                  rect.y + dstPaddingY * 2 - dstBorderSize * 2 + i * fontSize * rs.wh / rs.ah);
        if (i == currentIndex)
        {
            SDL_Rect selRect;
            selRect.x = rect.x + dstPaddingX;
            selRect.y = rect.y + dstPaddingY * 2 - dstBorderSize * 2 + i * selectionSurface->h * rs.wh / rs.ah;
            selRect.w = selectionSurface->w * rs.ww / rs.aw;
            selRect.h = selectionSurface->h * rs.wh / rs.ah;
            SDL_RenderCopy(renderer, selectionTexture, NULL, &selRect);
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
