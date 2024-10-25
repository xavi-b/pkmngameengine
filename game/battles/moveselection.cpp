#include "moveselection.h"

#include "game.h"
#include "renderutils.h"
#include "textspeech.h"

#include <SDL_ttf.h>
#include <iostream>

MoveSelection::MoveSelection(SDL_Renderer* renderer) : renderer(renderer)
{
    bgSurface        = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture        = SDL_CreateTextureFromSurface(renderer, bgSurface);
    selectionSurface = IMG_Load("resources/Graphics/Pictures/selarrow.png");
    selectionTexture = SDL_CreateTextureFromSurface(renderer, selectionSurface);
}

MoveSelection::~MoveSelection()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(selectionTexture);
    SDL_FreeSurface(selectionSurface);
}

void MoveSelection::init()
{
    reset();
}

void MoveSelection::update(Inputs const* inputs)
{
    auto const nonNullMovesResult = std::find(pkmn->getMoves().begin(), pkmn->getMoves().end(), nullptr);
    int const  nonNullMovesCount  = std::distance(pkmn->getMoves().begin(), nonNullMovesResult);

    if (inputs->A)
    {
        selected = true;
        if (Game::instance()->isDebug())
            std::cout << "MoveSelection selected: " << currentIndex << std::endl;
    }
    else if (inputs->B)
    {
        quit     = true;
        selected = false;
    }
    else if (inputs->right)
    {
        if (currentIndex % 2 == 0) // first column
        {
            size_t oldIndex = currentIndex;
            currentIndex    = (currentIndex + 1) % MaxMoveCount;
            if (currentIndex >= nonNullMovesCount)
                currentIndex = oldIndex;
        }
    }
    else if (inputs->left)
    {
        if (currentIndex % 2 == 1) // second column
        {
            size_t oldIndex = currentIndex;
            currentIndex    = (currentIndex - 1 + MaxMoveCount) % MaxMoveCount;
            if (currentIndex >= nonNullMovesCount)
                currentIndex = oldIndex;
        }
    }
    else if (inputs->down)
    {
        if (currentIndex < int(MaxMoveCount) - 2) // second
        {
            size_t oldIndex = currentIndex;
            currentIndex    = (currentIndex + 2) % MaxMoveCount;
            if (currentIndex >= nonNullMovesCount)
                currentIndex = oldIndex;
        }
    }
    else if (inputs->up)
    {
        if (currentIndex > 1) // second row
        {
            size_t oldIndex = currentIndex;
            currentIndex    = (currentIndex - 2 + MaxMoveCount) % MaxMoveCount;
            if (currentIndex >= nonNullMovesCount)
                currentIndex = oldIndex;
        }
    }
}

void MoveSelection::draw(Fps const* /*fps*/, RenderSizes rs)
{
    std::vector<std::string> movesTexts;
    for (size_t i = 0; i < pkmn->getMoves().size(); ++i)
    {
        if (pkmn->getMoves().at(i))
            movesTexts.push_back(pkmn->getMoves().at(i)->getDefinition()->getName());
    }

    int borderSize     = TextSpeech::TextBoxBorderSize;
    int dstBorderSizeX = borderSize * rs.ww / rs.aw;

    int height    = 2 /* lines */ * TextSpeech::TextBoxHeight;
    int dstHeight = height * rs.wh / rs.ah;

    int fontSize      = RenderUtils::TextSize;
    int dstTextHeight = fontSize * rs.wh / rs.ah;

    int dstPaddingY = (dstHeight - dstTextHeight * NElements) / (NElements + 1);
    int dstPaddingX = MoveSelection::TextPaddingX * rs.ww / rs.aw;

    int percentage    = 60;
    int dstMovesWidth = rs.ww * percentage / 100;
    int dstInfoWidth  = rs.ww - dstMovesWidth;

    int selectionSurfaceWidth     = MoveSelection::ArrowSize;
    int dstSelectionSurfaceWidth  = selectionSurfaceWidth * rs.ww / rs.aw;
    int selectionSurfaceHeight    = selectionSurface->h;
    int dstSelectionSurfaceHeight = selectionSurfaceHeight * rs.wh / rs.ah;
    int dstSelectionTotalWidth    = (dstMovesWidth - 2 * dstBorderSizeX - dstPaddingX) / 2;

    int textAdjustX = 2 * rs.ww / rs.aw;
    int textAdjustY = -4 * rs.wh / rs.ah;

    SDL_Rect rect;

    rect.w = dstMovesWidth;
    rect.x = 0;
    rect.h = dstHeight;
    rect.y = rs.wh - rect.h;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int i = 0;
    for (auto const& text : movesTexts)
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

    rect.w = dstInfoWidth;
    rect.x = dstMovesWidth;
    rect.h = dstHeight;
    rect.y = rs.wh - rect.h;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);
}

bool MoveSelection::isSelected() const
{
    return selected;
}

void MoveSelection::reset()
{
    currentIndex = 0;
    selected     = false;
    quit         = false;
}

int MoveSelection::selectedIndex() const
{
    return currentIndex;
}

bool MoveSelection::shouldQuit() const
{
    return quit;
}

void MoveSelection::setPkmn(Pkmn::PkmnPtr const& pkmn)
{
    this->pkmn = pkmn;
}
