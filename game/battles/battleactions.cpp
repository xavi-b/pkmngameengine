#include "battleactions.h"

#include "game.h"
#include "renderutils.h"
#include "textspeech.h"

#include <SDL_ttf.h>
#include <iostream>

std::string BattleActions::TypeToString(Type e)
{
    switch (e)
    {
    case FIGHT:
        return "FIGHT";
    case BAG:
        return "BAG";
    case PKNMS:
        return "PKNMS";
    case RUN:
        return "RUN";
    default:
        return "???";
    }
}

BattleActions::BattleActions(SDL_Renderer* renderer) : renderer(renderer)
{
    bgSurface        = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture        = SDL_CreateTextureFromSurface(renderer, bgSurface);
    selectionSurface = IMG_Load("resources/Graphics/Pictures/selarrow.png");
    selectionTexture = SDL_CreateTextureFromSurface(renderer, selectionSurface);
}

BattleActions::~BattleActions()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(selectionTexture);
    SDL_FreeSurface(selectionSurface);
}

void BattleActions::init()
{
    reset();
}

void BattleActions::update(Inputs const* inputs)
{
    if (inputs->A)
    {
        selected = true;
    }
    else if (inputs->right)
    {
        if (currentIndex % 2 == 0) // first column
            currentIndex = (currentIndex + 1) % TypeCount;
        if (Game::instance()->isDebug())
            std::cout << "BattleActions: " << currentIndex << std::endl;
    }
    else if (inputs->left)
    {
        if (currentIndex % 2 == 1) // second column
            currentIndex = (currentIndex - 1 + TypeCount) % TypeCount;
        if (Game::instance()->isDebug())
            std::cout << "BattleActions: " << currentIndex << std::endl;
    }
    else if (inputs->down)
    {
        if (currentIndex < int(TypeCount) - 2) // second row
            currentIndex = (currentIndex + 2) % TypeCount;
        if (Game::instance()->isDebug())
            std::cout << "BattleActions: " << currentIndex << std::endl;
    }
    else if (inputs->up)
    {
        if (currentIndex > 1) // second row
            currentIndex = (currentIndex - 2 + TypeCount) % TypeCount;
        if (Game::instance()->isDebug())
            std::cout << "BattleActions: " << currentIndex << std::endl;
    }
}

void BattleActions::draw(Fps const* /*fps*/, RenderSizes rs)
{
    std::vector<std::string> texts;
    for (size_t i = Type::FIGHT; i < TypeCount; ++i)
        texts.push_back(TypeToString(static_cast<Type>(i)));

    int borderSize     = 14;
    int dstBorderSizeX = borderSize * rs.ww / rs.aw;

    int fontSize  = RenderUtils::TextSize;
    int height    = 2 /* lines */ * TextSpeech::TextBoxSize;
    int dstHeight = height * rs.wh / rs.ah;

    int maxW = 0;
    int h;
    for (auto const& text : texts)
    {
        int w;
        TTF_SetFontSize(RenderUtils::instance()->font, fontSize);
        TTF_SizeText(RenderUtils::instance()->font, text.c_str(), &w, &h);
        if (w > maxW)
            maxW = w;
    }
    int dstH = h * rs.wh / rs.ah;

    size_t nElements = 2;

    int selectionSurfaceWidth     = selectionSurface->w;
    int dstSelectionSurfaceWidth  = selectionSurfaceWidth * rs.ww / rs.aw;
    int selectionSurfaceHeight    = selectionSurface->h;
    int dstSelectionSurfaceHeight = selectionSurfaceHeight * rs.wh / rs.ah;
    int selectionTotalWidth       = selectionSurface->w + maxW;
    int dstSelectionTotalWidth    = selectionTotalWidth * rs.ww / rs.aw;

    int dstPaddingY     = (dstHeight - dstH * nElements) / (nElements + 1);
    int dstPaddingX     = 10 * rs.ww / rs.aw;
    int dstTextBoxWidth = 2 * dstBorderSizeX + nElements * dstSelectionTotalWidth + (nElements + 1) * dstPaddingX;

    int textAdjustX = 2 * rs.ww / rs.aw;
    int textAdjustY = -2 * rs.wh / rs.ah;

    SDL_Rect rect;
    rect.w = dstTextBoxWidth;
    rect.x = rs.ww - rect.w;
    rect.h = dstHeight;
    rect.y = rs.wh - rect.h;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int i = 0;
    for (auto const& text : texts)
    {
        RenderUtils::drawGreyText(renderer,
                                  rs,
                                  text,
                                  fontSize,
                                  rect.x + dstBorderSizeX + dstPaddingX + dstSelectionSurfaceWidth
                                      + (i % 2) * (dstSelectionTotalWidth + dstPaddingX) + textAdjustX,
                                  rect.y + dstPaddingY + (i / 2) * (dstH + dstPaddingY) + textAdjustY);
        if (i == currentIndex)
        {
            SDL_Rect selRect;
            selRect.x = rect.x + dstBorderSizeX + dstPaddingX + (i % 2) * (dstSelectionTotalWidth + dstPaddingX);
            selRect.y = rect.y + dstPaddingY + (i / 2) * (dstH + dstPaddingY + (dstH - dstSelectionSurfaceHeight) / 2);
            selRect.w = dstSelectionSurfaceWidth;
            selRect.h = dstSelectionSurfaceHeight;
            SDL_RenderCopy(renderer, selectionTexture, NULL, &selRect);
        }
        ++i;
    }
}

bool BattleActions::isFinished() const
{
    return selected;
}

void BattleActions::reset()
{
    currentIndex = 0;
    selected     = false;
}

int BattleActions::selectedIndex() const
{
    return currentIndex;
}

BattleActions::Type BattleActions::selectedAction() const
{
    return static_cast<Type>(currentIndex);
}
