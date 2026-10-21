#include "singlebattleplayerpkmnsphase.h"

#include "../abilitybattle.h"
#include "../entryhazards.h"
#include "../moveeffects.h"
#include "../moveresolver.h"
#include "../singlebattlescene.h"
#include "../statuscondition.h"
#include "battles/battleactions.h"
#include "game.h"
#include "itemutils.h"
#include "textspeech.h"
#include "utils.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

SingleBattlePlayerPkmnsPhase::SingleBattlePlayerPkmnsPhase(SingleBattleScene&                     scene,
                                                           std::shared_ptr<SingleBattleSceneData> data,
                                                           SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattlePlayerPkmnsPhase::showEnterSpeech(std::string const& text)
{
    sendOutStarted      = false;
    hazardsDone         = false;
    hazardFainted       = false;
    pursuitResolved     = false;
    pursuitConsumedTurn = false;
    hazardSpeech.reset();
    pkmnEnterSpeech = std::make_unique<TextSpeech>(renderer);
    pkmnEnterSpeech->setTexts({text});
    pkmnEnterSpeech->start();
}

void SingleBattlePlayerPkmnsPhase::update(Inputs const* inputs)
{
    if (pkmnEnterSpeech)
    {
        inputs->setHandler(pkmnEnterSpeech->inputId());
        pkmnEnterSpeech->update(inputs);
        if (pkmnEnterSpeech->shouldClose())
            pkmnEnterSpeech.reset();
        return;
    }

    if (hazardSpeech)
    {
        inputs->setHandler(hazardSpeech->inputId());
        hazardSpeech->update(inputs);
        if (!hazardSpeech->shouldClose())
            return;
        hazardSpeech.reset();
    }

    auto shedShell = [](Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler) {
        return ItemUtils::activeHeldId(pkmn, &battler) == "SHEDSHELL";
    };
    bool canLeave = shedShell(data->playerPkmn, data->playerBattler);
    // Mean Look blocks a menu switch. Pivots and Baton Pass still leave. Fairy Lock blocks every switch.
    bool meanLook = data->playerBattler.trapped && !canLeave && data->playerVoluntarySwitch
                    && !data->playerBatonPass.pending;
    bool lockedIn = data->fairyLock || meanLook;
    if (!trapChecked && data->playerPkmn && !data->playerPkmn->isKO() && lockedIn)
    {
        trapChecked = true;
        showEnterSpeech(lc::translate("It can't be switched out!"));
        return;
    }
    if (trapChecked && lockedIn && data->playerPkmn && !data->playerPkmn->isKO())
    {
        data->state = ACTIONS;
        return;
    }

    if (!sendOutStarted)
    {
        if (!pursuitResolved)
        {
            pursuitResolved = true;
            // Use the foe's locked command only — do not invent a move at switch time.
            // Gen 7+: Pursuit only on voluntary switch / Baton Pass (not U-turn / ejects).
            data->playerVoluntarySwitch = false;
            if (data->encounterMove && data->encounterMove->getDefinition()
                && data->encounterMove->getDefinition()->getFunctionCode()
                       == MoveDef::FunctionCode::PursueSwitchingFoe
                && data->opponentPkmn && !data->opponentPkmn->isKO() && data->playerPkmn && !data->playerPkmn->isKO())
            {
                data->pursuitStrike = true;
                auto pursued        = MoveResolver::resolve(*data, false, data->encounterMove);
                data->pursuitStrike = false;
                data->encounterMove.reset();
                pursuitConsumedTurn = true;
                if (data->playerPkmn && data->playerPkmn->isKO())
                    data->playerFaintedThisTurn = true;
                if (data->opponentPkmn && data->opponentPkmn->isKO())
                    data->opponentFaintedThisTurn = true;
                if (!pursued.messages.empty())
                {
                    hazardSpeech = std::make_unique<TextSpeech>(renderer);
                    hazardSpeech->setTexts(pursued.messages);
                    hazardSpeech->start();
                    return;
                }
            }
        }

        if (!data->newSelectedPkmn || data->newSelectedPkmn->isKO())
        {
            data->state = data->playerPkmn && !data->playerPkmn->isKO() ? ACTIONS : PKMNS;
            return;
        }

        scene.setPlayerPkmn(data->newSelectedPkmn, true);
        data->newSelectedPkmn.reset();
        battleUi()->startSwitchSendOut();
        sendOutStarted = true;
        return;
    }

    if (!battleUi()->isSwitchSendOutDone())
    {
        battleUi()->incrementTicks();
        return;
    }

    if (!hazardsDone)
    {
        hazardsDone = true;
        auto hazard = applyEntryHazards(*data, true);
        hazardFainted = hazard.fainted;
        if (!hazard.messages.empty())
        {
            hazardSpeech = std::make_unique<TextSpeech>(renderer);
            hazardSpeech->setTexts(hazard.messages);
            hazardSpeech->start();
            return;
        }
    }

    if (hazardFainted)
    {
        data->playerHealingWish = false;
        data->playerLunarDance  = false;
        if (!playFaint(true))
            return;

        bool hasNext = false;
        for (auto const& pkmn : Game::instance()->data.player.pkmns)
        {
            if (pkmn && !pkmn->isKO())
                hasNext = true;
        }
        data->playerPkmn = nullptr;
        data->state      = hasNext ? PKMNS : END;
        return;
    }

    // Hazards → Healing Wish / Lunar Dance → abilities.
    if (data->playerPkmn && data->playerHealingWish)
    {
        data->playerHealingWish = false;
        data->playerPkmn->resetHP();
        data->playerPkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
        if (data->playerLunarDance)
        {
            data->playerLunarDance = false;
            for (auto& slot : data->playerPkmn->getMoves())
            {
                if (slot && slot->getDefinition())
                    slot->setCurrentPP(slot->maxPP());
            }
        }
        data->pendingAbilityMessages.push_back(
            (boost::format(lc::translate("%1% was healed by the wish!")) % data->playerPkmn->getDisplayName()).str());
    }
    data->playerLunarDance = false;

    {
        AbilityEvent event;
        event.playerSide = true;
        triggerAbility(*data, AbilityMoment::SwitchIn, event);
        data->pendingAbilityMessages.insert(data->pendingAbilityMessages.end(), event.messages.begin(),
                                            event.messages.end());
    }

    if (pursuitConsumedTurn)
    {
        data->state = WEATHER;
        return;
    }

    // Residual faint replace opens a new turn at command select.
    if (data->replaceAfterResidual)
    {
        data->replaceAfterResidual = false;
        data->state                = ACTIONS;
        return;
    }

    // Mid-turn: only unmoved foes still act; otherwise finish the turn.
    if (data->opponentBattler.movedThisTurn || !data->opponentPkmn || data->opponentPkmn->isKO())
    {
        data->state = WEATHER;
        return;
    }

    data->playerSwitchQueued = false;

    if (data->opponentAction == BattleActions::PKMNS && !data->opponentSwitchDone)
    {
        data->state = OPPONENT_PKMNS;
        return;
    }
    if (data->playerItemQueued)
    {
        data->state = PLAYER_ITEMS;
        return;
    }

    switch (data->opponentAction)
    {
    case BattleActions::BAG:
        data->state = data->opponentItemDone ? WEATHER : OPPONENT_ITEMS;
        break;
    case BattleActions::PKMNS:
        data->state = WEATHER;
        break;
    case BattleActions::RUN:
        data->state = OPPONENT_RUN;
        break;
    case BattleActions::MOVES:
    default:
        data->state = OPPONENT_MOVES;
        break;
    }
}

void SingleBattlePlayerPkmnsPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (pkmnEnterSpeech)
    {
        pkmnEnterSpeech->draw(fps, rs);
        return;
    }
    if (hazardSpeech)
        hazardSpeech->draw(fps, rs);
}
