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

void TextQuestion::update(Inputs const* inputs)
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

void TextQuestion::draw(Fps const* /*fps*/, RenderSizes rs, Position pos)
{
    int borderSize     = TextSpeech::TextBoxBorderSize;
    int dstBorderSizeX = borderSize * rs.ww / rs.aw;
    int dstBorderSizeY = borderSize * rs.wh / rs.ah;

    int fontSize      = RenderUtils::TextSize;
    int dstTextHeight = fontSize * rs.wh / rs.ah;

    int maxW = 0;
    for (auto const& text : texts)
    {
        int w, h;
        TTF_SetFontSize(RenderUtils::instance()->font, fontSize);
        TTF_SizeText(RenderUtils::instance()->font, text.c_str(), &w, &h);
        if (w > maxW)
            maxW = w;
    }

    size_t nElements = texts.size();

    int padding     = RenderUtils::TextPadding;
    int dstPaddingX = padding * rs.ww / rs.aw;
    int dstPaddingY = padding * rs.wh / rs.ah;

    int selectionSurfaceWidth     = TextQuestion::ArrowSize;
    int dstSelectionSurfaceWidth  = selectionSurfaceWidth * rs.ww / rs.aw;
    int selectionSurfaceHeight    = selectionSurface->h;
    int dstSelectionSurfaceHeight = selectionSurfaceHeight * rs.wh / rs.ah;
    int selectionTotalWidth       = TextQuestion::ArrowSize + maxW;
    int dstSelectionTotalWidth    = selectionTotalWidth * rs.ww / rs.aw;

    int height    = TextSpeech::TextBoxBorderSize * 2 + fontSize * nElements + TextPaddingX * (nElements + 1);
    int dstHeight = height * rs.wh / rs.ah;
    int dstWidth  = dstSelectionTotalWidth + dstPaddingX * 2 + dstBorderSizeX * 2;

    int dstSpeechHeight = (2 * (TextSpeech::TextBoxHeight * rs.wh / rs.ah));

    SDL_Rect rect;
    rect.w = dstWidth;
    rect.x = pos == Left ? 0 : rs.ww - rect.w;
    rect.h = dstHeight;
    rect.y = rs.wh - rect.h - dstSpeechHeight;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int textAdjustX = 2 * rs.ww / rs.aw;
    int textAdjustY = -2 * rs.wh / rs.ah;

    int i = 0;
    for (auto const& text : texts)
    {
        RenderUtils::drawGreyText(renderer,
                                  rs,
                                  text,
                                  fontSize,
                                  rect.x + dstBorderSizeX + dstPaddingX + dstSelectionSurfaceWidth + textAdjustX,
                                  rect.y + dstBorderSizeY + dstPaddingY + i * (dstTextHeight + dstPaddingY)
                                      + textAdjustY);
        if (i == currentIndex)
        {
            SDL_Rect selRect;
            selRect.x = rect.x + dstBorderSizeX + dstPaddingX;
            selRect.y = rect.y + dstBorderSizeY + dstPaddingY + i * (dstTextHeight + dstPaddingY)
                      + (dstTextHeight - dstSelectionSurfaceHeight) / 2;
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

void TextQuestion::setTexts(std::vector<std::string> const& texts)
{
    this->texts = texts;
}

int TextQuestion::selectedIndex() const
{
    return currentIndex;
}
