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
    case MOVES:
        return "FIGHT";
    case BAG:
        return "BAG";
    case PKMNS:
        return "PKMNS";
    case RUN:
        return "RUN";
    case __SIZE:
        return "__SIZE";
    }
    return "???";
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
    for (size_t i = Type::MOVES; i < TypeCount; ++i)
        texts.push_back(TypeToString(static_cast<Type>(i)));

    int borderSize     = TextSpeech::TextBoxBorderSize;
    int dstBorderSizeX = borderSize * rs.ww / rs.aw;

    int height    = 2 /* lines */ * TextSpeech::TextBoxHeight;
    int dstHeight = height * rs.wh / rs.ah;

    int fontSize      = RenderUtils::TextSize;
    int dstTextHeight = fontSize * rs.wh / rs.ah;

    int dstPaddingY = (dstHeight - dstTextHeight * NElements) / (NElements + 1);
    int dstWidth    = BattleActions::TextBoxWidth * rs.ww / rs.aw;
    int dstPaddingX = BattleActions::TextPaddingX * rs.ww / rs.aw;

    int selectionSurfaceWidth     = BattleActions::ArrowSize;
    int dstSelectionSurfaceWidth  = selectionSurfaceWidth * rs.ww / rs.aw;
    int selectionSurfaceHeight    = selectionSurface->h;
    int dstSelectionSurfaceHeight = selectionSurfaceHeight * rs.wh / rs.ah;
    int selectionTotalWidth       = BattleActions::ArrowSize + BattleActions::TextWidth;
    int dstSelectionTotalWidth    = selectionTotalWidth * rs.ww / rs.aw;

    int textAdjustX = 2 * rs.ww / rs.aw;
    int textAdjustY = -4 * rs.wh / rs.ah;

    SDL_Rect rect;
    rect.w = dstWidth;
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
                                  rect.y + dstPaddingY + (i / 2) * (dstTextHeight + dstPaddingY) + textAdjustY);
        if (i == currentIndex)
        {
            SDL_Rect selRect;
            selRect.x = rect.x + dstBorderSizeX + dstPaddingX + (i % 2) * (dstSelectionTotalWidth + dstPaddingX);
            selRect.y = rect.y + dstPaddingY + (i / 2) * (dstTextHeight + dstPaddingY)
                      + (dstTextHeight - dstSelectionSurfaceHeight) / 2;
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
    selected = false;
}

int BattleActions::selectedIndex() const
{
    return currentIndex;
}

BattleActions::Type BattleActions::selectedAction() const
{
    return static_cast<Type>(currentIndex);
}
