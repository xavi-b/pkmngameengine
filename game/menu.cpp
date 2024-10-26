#include "menu.h"

#include "renderutils.h"
#include "textspeech.h"
#include "utils.h"

#include <SDL_ttf.h>
#include <iostream>

std::string Menu::SelectionToString(Selection e)
{
    switch (e)
    {
    case PKMNS:
        return lc::translate("PKMNS");
    case BAG:
        return lc::translate("BAG");
    case SAVE:
        return lc::translate("SAVE");
    case QUIT:
        return lc::translate("QUIT");
    case __SIZE:
        return "__SIZE";
    }
    return "???";
}

Menu::Menu(SDL_Renderer* renderer) : renderer(renderer)
{
    bgSurface        = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture        = SDL_CreateTextureFromSurface(renderer, bgSurface);
    selectionSurface = IMG_Load("resources/Graphics/Pictures/selarrow.png");
    selectionTexture = SDL_CreateTextureFromSurface(renderer, selectionSurface);
}

Menu::~Menu()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(selectionTexture);
    SDL_FreeSurface(selectionSurface);
}

void Menu::init()
{
    reset();
}

void Menu::update(Inputs const* inputs)
{
    if (inputs->A)
    {
        selected = true;
        finished = true;
    }
    else if (inputs->B || inputs->start)
    {
        selected = false;
        finished = true;
    }
    else if (inputs->up)
    {
        if (SelectionCount > 0)
            currentSelection = static_cast<Selection>((currentSelection - 1 + SelectionCount) % SelectionCount);
    }
    else if (inputs->down)
    {
        if (SelectionCount > 0)
            currentSelection = static_cast<Selection>((currentSelection + 1) % SelectionCount);
    }
}

void Menu::draw(Fps const* /*fps*/, RenderSizes rs)
{
    int borderSize     = TextSpeech::TextBoxBorderSize;
    int dstBorderSizeX = borderSize * rs.ww / rs.aw;
    int dstBorderSizeY = borderSize * rs.wh / rs.ah;

    int fontSize      = RenderUtils::TextSize;
    int dstTextHeight = fontSize * rs.wh / rs.ah;

    std::vector<std::string> texts;
    for (size_t i = 0; i < SelectionCount; ++i)
        texts.push_back(SelectionToString(static_cast<Selection>(i)));

    int maxW = 0;
    for (auto const& text : texts)
    {
        int w, h;
        TTF_SetFontSize(RenderUtils::instance()->font, fontSize);
        TTF_SizeText(RenderUtils::instance()->font, text.c_str(), &w, &h);
        if (w > maxW)
            maxW = w;
    }

    size_t nElements = SelectionCount;

    int padding     = RenderUtils::TextPadding;
    int dstPaddingX = padding * rs.ww / rs.aw;
    int dstPaddingY = padding * rs.wh / rs.ah;

    int selectionSurfaceWidth     = Menu::ArrowSize;
    int dstSelectionSurfaceWidth  = selectionSurfaceWidth * rs.ww / rs.aw;
    int selectionSurfaceHeight    = selectionSurface->h;
    int dstSelectionSurfaceHeight = selectionSurfaceHeight * rs.wh / rs.ah;
    int selectionTotalWidth       = Menu::ArrowSize + maxW;
    int dstSelectionTotalWidth    = selectionTotalWidth * rs.ww / rs.aw;

    int height    = TextSpeech::TextBoxBorderSize * 2 + fontSize * nElements + TextPaddingX * (nElements + 1);
    int dstHeight = height * rs.wh / rs.ah;
    int dstWidth  = dstSelectionTotalWidth + dstPaddingX * 2 + dstBorderSizeX * 2;

    SDL_Rect rect;
    rect.w = dstWidth;
    rect.x = rs.ww - rect.w;
    rect.h = dstHeight;
    rect.y = 0;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int textAdjustX = 2 * rs.ww / rs.aw;
    int textAdjustY = -4 * rs.wh / rs.ah;

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
        if (i == currentSelection)
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

bool Menu::isFinished() const
{
    return finished;
}

bool Menu::isSelected() const
{
    return selected;
}

void Menu::reset()
{
    currentSelection = static_cast<Selection>(0);
    selected         = false;
    finished         = false;
}

Menu::Selection Menu::selection() const
{
    return currentSelection;
}
