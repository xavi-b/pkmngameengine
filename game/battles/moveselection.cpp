#include "moveselection.h"

#include "game.h"
#include "itemutils.h"
#include "movedef.h"
#include "renderutils.h"
#include "scenes/battle/moveeffects.h"
#include "textspeech.h"
#include "utils.h"

#include <SDL_ttf.h>
#include <algorithm>
#include <boost/format.hpp>
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

bool MoveSelection::moveUsable(size_t index) const
{
    if (!pkmn || index >= pkmn->getMoves().size())
        return false;
    auto move = pkmn->getMoves().at(index);
    if (!move || !move->getDefinition())
        return false;
    auto def = move->getDefinition();
    if (!userBattler)
        return move->getCurrentPP() > 0;
    if (userBattler->recharging)
        return true; // selection is ignored; resolver forces the recharge turn
    if (move->getCurrentPP() == 0)
        return false;
    if (userBattler->disableTurns > 0 && userBattler->disabledMoveId == def->getId())
        return false;
    if (userBattler->encoreTurns > 0 && !userBattler->encoreMoveId.empty())
    {
        // Encore ends when the locked move has 0 PP.
        bool encoreAlive = false;
        for (auto const& slot : pkmn->getMoves())
        {
            if (slot && slot->getDefinition() && slot->getDefinition()->getId() == userBattler->encoreMoveId
                && slot->getCurrentPP() > 0)
            {
                encoreAlive = true;
                break;
            }
        }
        if (encoreAlive && userBattler->encoreMoveId != def->getId())
            return false;
    }
    if (userBattler->charging)
    {
        if (!isTwoTurnChargingMove(def->getFunctionCode()))
            return false;
        if (!userBattler->lastMoveId.empty() && userBattler->lastMoveId != def->getId())
            return false;
    }
    if (userBattler->torment && userBattler->lastMoveId == def->getId())
        return false;
    if (userBattler->tauntTurns > 0 && def->getCategory() == MoveDef::STATUS)
        return false;
    if (userBattler->soundBlockTurns > 0 && def->hasFlag(MoveDef::Flag::Sound))
        return false;
    bool gorilla = !userBattler->abilitySuppressed && userBattler->abilityId == "GORILLATACTICS";
    bool klutz   = !userBattler->abilitySuppressed && userBattler->abilityId == "KLUTZ";
    std::string held;
    if (!userBattler->itemsBlocked && !klutz)
        held = ItemUtils::activeHeldId(pkmn, userBattler);
    bool choiceLocked = gorilla
                     || held == "CHOICEBAND" || held == "CHOICESPECS" || held == "CHOICESCARF";
    if (choiceLocked && !userBattler->choiceLock.empty() && userBattler->choiceLock != def->getId())
        return false;
    if (held == "ASSAULTVEST" && def->getCategory() == MoveDef::STATUS)
        return false;
    if (userBattler->multiTurnLock > 0)
    {
        if (!userBattler->rolloutMoveId.empty() && userBattler->rolloutMoveId != def->getId())
            return false;
        if (!userBattler->rampageMoveId.empty() && userBattler->rampageMoveId != def->getId())
            return false;
    }
    if (userBattler->bideActive
        && def->getFunctionCode() != MoveDef::FunctionCode::MultiTurnAttackBideThenReturnDoubleDamage)
        return false;
    if (userBattler->uproarTurns > 0 && !userBattler->uproarMoveId.empty()
        && userBattler->uproarMoveId != def->getId())
        return false;
    if (sceneGravity && gravityBlocksMove(def->getFunctionCode()))
        return false;
    if (foeBattler && foeBattler->imprison && foe)
    {
        for (auto const& slot : foe->getMoves())
        {
            if (slot && slot->getDefinition() && slot->getDefinition()->getId() == def->getId())
                return false;
        }
    }
    return true;
}

bool MoveSelection::anyMoveUsable() const
{
    if (!pkmn)
        return false;
    for (size_t i = 0; i < pkmn->getMoves().size(); ++i)
    {
        if (moveUsable(i))
            return true;
    }
    return false;
}

void MoveSelection::update(Inputs const* inputs)
{
    auto const nonNullMovesResult = std::find(pkmn->getMoves().begin(), pkmn->getMoves().end(), nullptr);
    int const  nonNullMovesCount  = std::distance(pkmn->getMoves().begin(), nonNullMovesResult);

    if (inputs->A)
    {
        if (moveUsable(static_cast<size_t>(currentIndex)))
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
    std::vector<bool>        usableFlags;
    for (size_t i = 0; i < pkmn->getMoves().size(); ++i)
    {
        if (pkmn->getMoves().at(i))
        {
            movesTexts.push_back(pkmn->getMoves().at(i)->getDefinition()->getName());
            usableFlags.push_back(moveUsable(i));
        }
    }

    int borderSize     = TextSpeech::TextBoxBorderSize;
    int dstBorderSizeX = borderSize * rs.ww / rs.aw;

    int height    = 2 /* lines */ * TextSpeech::TextBoxHeight;
    int dstHeight = height * rs.wh / rs.ah;

    int fontSize      = RenderUtils::TextSize * 3 / 4;
    int dstTextHeight = fontSize * rs.wh / rs.ah;

    int dstPaddingY = (dstHeight - dstTextHeight * NElements) / (NElements + 1);
    int dstPaddingX = MoveSelection::TextPaddingX * rs.ww / rs.aw;

    int percentage    = 70;
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
        int x = rect.x + dstBorderSizeX + dstPaddingX + dstSelectionSurfaceWidth
              + (i % 2) * (dstSelectionTotalWidth + dstPaddingX) + textAdjustX;
        int y = rect.y + dstPaddingY + (i / 2) * (dstTextHeight + dstPaddingY) + textAdjustY;
        if (usableFlags.at(static_cast<size_t>(i)))
            RenderUtils::drawGreyText(renderer, rs, text, fontSize, x, y);
        else
        {
            SDL_Color color   = {160, 160, 160, 255};
            SDL_Color bgColor = {220, 220, 220, 255};
            RenderUtils::drawText(renderer, rs, text, color, bgColor, fontSize, x, y, -1);
        }
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

    auto currentMove = pkmn->getMoves().at(currentIndex);

    if (currentMove)
    {
        RenderUtils::drawGreyText(renderer,
                                  rs,
                                  "PP",
                                  fontSize,
                                  rect.x + dstBorderSizeX + dstPaddingX + textAdjustX,
                                  rect.y + dstPaddingY + textAdjustY);

        auto formatPP =
            boost::format("%1%/%2%") % currentMove->getCurrentPP() % currentMove->maxPP();
        RenderUtils::drawGreyTextRightAligned(renderer,
                                              rs,
                                              formatPP.str(),
                                              fontSize,
                                              rect.x + rect.w - dstBorderSizeX - dstPaddingX - textAdjustX,
                                              rect.y + dstPaddingY + textAdjustY);

        RenderUtils::drawGreyText(renderer,
                                  rs,
                                  "Type/" + currentMove->getDefinition()->getType(),
                                  fontSize,
                                  rect.x + dstBorderSizeX + dstPaddingX + textAdjustX,
                                  rect.y + dstPaddingY + dstTextHeight + dstPaddingY + textAdjustY);
    }
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

void MoveSelection::setGravity(bool gravity)
{
    sceneGravity = gravity;
}

void MoveSelection::setBattlers(BattleBattler const* userBattler, Pkmn::PkmnPtr const& foe,
                                BattleBattler const* foeBattler)
{
    this->userBattler = userBattler;
    this->foe         = foe;
    this->foeBattler  = foeBattler;
}
