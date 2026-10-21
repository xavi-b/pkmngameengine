#include "singlebattleopponentpkmnsphase.h"

#include "../abilitybattle.h"
#include "../entryhazards.h"
#include "../moveresolver.h"
#include "../singlebattlescene.h"
#include "../trainerscenedata.h"
#include "game.h"
#include "itemutils.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

SingleBattleOpponentPkmnsPhase::SingleBattleOpponentPkmnsPhase(SingleBattleScene&                     scene,
                                                               std::shared_ptr<SingleBattleSceneData> data,
                                                               SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleOpponentPkmnsPhase::preupdate()
{
    speech.reset();
    throwStarted  = false;
    hazardsDone   = false;
    hazardFainted = false;
    pursuitDone   = false;
    trapChecked   = false;
    trapBlocked   = false;
}

void SingleBattleOpponentPkmnsPhase::update(Inputs const* inputs)
{
    auto trainerData = std::dynamic_pointer_cast<TrainerSceneData>(data);
    if (!trainerData)
    {
        data->state = WEATHER;
        return;
    }

    if (speech)
    {
        inputs->setHandler(speech->inputId());
        speech->update(inputs);
        if (!speech->shouldClose())
            return;
        speech.reset();
    }

    if (!throwStarted)
    {
        if (!trapChecked && data->opponentPkmn && !data->opponentPkmn->isKO())
        {
            trapChecked = true;
            bool shed   = ItemUtils::activeHeldId(data->opponentPkmn, &data->opponentBattler) == "SHEDSHELL";
            bool meanLook = data->opponentBattler.trapped && !shed && data->opponentVoluntarySwitch
                            && !data->opponentBatonPass.pending;
            if (data->fairyLock || meanLook)
            {
                trapBlocked = true;
                speech      = std::make_unique<TextSpeech>(renderer);
                speech->setTexts({lc::translate("It can't be switched out!")});
                speech->start();
                return;
            }
        }
        if (trapBlocked)
        {
            trainerData->nextOpponentPkmn.reset();
            trainerData->shouldSwitchOpponentPkmn = false;
            data->opponentVoluntarySwitch         = false;
            if (data->playerMove && data->playerPkmn && !data->playerPkmn->isKO() && !data->playerBattler.movedThisTurn)
                data->state = PLAYER_MOVES;
            else
                data->state = WEATHER;
            return;
        }

        if (!trainerData->nextOpponentPkmn)
        {
            trainerData->shouldSwitchOpponentPkmn = false;
            data->state                           = trainerData->opponentPkmn && !trainerData->opponentPkmn->isKO()
                                                        ? WEATHER
                                                        : EXPERIENCE;
            return;
        }

        if (!pursuitDone)
        {
            pursuitDone = true;
            bool playerHasNotMoved = !data->playerBattler.movedThisTurn;
            data->opponentVoluntarySwitch = false;
            bool pursuit = playerHasNotMoved && data->playerMove && data->playerMove->getDefinition()
                        && data->playerMove->getDefinition()->getFunctionCode() == MoveDef::FunctionCode::PursueSwitchingFoe
                        && data->playerPkmn && !data->playerPkmn->isKO() && data->opponentPkmn && !data->opponentPkmn->isKO();
            if (pursuit)
            {
                data->pursuitStrike = true;
                auto pursued        = MoveResolver::resolve(*data, true, data->playerMove);
                data->pursuitStrike = false;
                data->playerMove.reset();
                if (data->playerPkmn && data->playerPkmn->isKO())
                    data->playerFaintedThisTurn = true;
                if (data->opponentPkmn && data->opponentPkmn->isKO())
                    data->opponentFaintedThisTurn = true;
                if (!pursued.messages.empty())
                {
                    speech = std::make_unique<TextSpeech>(renderer);
                    speech->setTexts(pursued.messages);
                    speech->start();
                    return;
                }
            }
        }

        scene.setOpponentPkmn(trainerData->nextOpponentPkmn, true);
        trainerData->nextOpponentPkmn.reset();
        trainerData->defeatedOpponentPkmnName.clear();
        battleUi()->startFoeSwitchSendOut();
        throwStarted = true;
        return;
    }

    if (!battleUi()->isFoeSendOutDone())
    {
        battleUi()->incrementTicks();
        return;
    }

    if (!hazardsDone)
    {
        hazardsDone = true;
        auto hazard = applyEntryHazards(*data, false);
        hazardFainted = hazard.fainted;
        if (!hazard.messages.empty())
        {
            speech = std::make_unique<TextSpeech>(renderer);
            speech->setTexts(hazard.messages);
            speech->start();
            return;
        }
    }

    if (hazardFainted && !playFaint(false))
        return;

    if (hazardFainted)
    {
        data->opponentHealingWish = false;
        data->opponentLunarDance  = false;
        Pkmn::PkmnPtr next;
        if (trainerData->opponentTrainer)
        {
            for (auto const& pkmn : trainerData->opponentTrainer->pkmns)
            {
                if (pkmn && !pkmn->isKO() && pkmn != data->opponentPkmn)
                {
                    next = pkmn;
                    break;
                }
            }
        }
        if (next)
        {
            trainerData->nextOpponentPkmn = next;
            speech                        = std::make_unique<TextSpeech>(renderer);
            speech->setTexts(
                {(boost::format(lc::translate("Trainer %1% sent out %2% !"))
                  % (trainerData->opponentTrainer ? trainerData->opponentTrainer->name : std::string())
                  % predictedIllusionName(next, false, *data))
                     .str()});
            speech->start();
            throwStarted  = false;
            hazardsDone   = false;
            hazardFainted = false;
            return;
        }

        trainerData->shouldSwitchOpponentPkmn = false;
        data->expGained.clear();
        data->state = EXPERIENCE;
        return;
    }

    if (data->opponentPkmn && data->opponentHealingWish)
    {
        data->opponentHealingWish = false;
        data->opponentPkmn->resetHP();
        data->opponentPkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
        if (data->opponentLunarDance)
        {
            data->opponentLunarDance = false;
            for (auto& slot : data->opponentPkmn->getMoves())
            {
                if (slot && slot->getDefinition())
                    slot->setCurrentPP(slot->maxPP());
            }
        }
        data->pendingAbilityMessages.push_back(
            (boost::format(lc::translate("%1% was healed by the wish!")) % data->opponentPkmn->getDisplayName()).str());
    }
    data->opponentLunarDance = false;

    {
        AbilityEvent event;
        event.playerSide = false;
        triggerAbility(*data, AbilityMoment::SwitchIn, event);
        data->pendingAbilityMessages.insert(data->pendingAbilityMessages.end(), event.messages.begin(),
                                            event.messages.end());
    }

    trainerData->shouldSwitchOpponentPkmn = false;
    trainerData->opponentSwitchAnnounced  = false;
    data->opponentSwitchDone              = true;
    if (data->playerSwitchQueued)
    {
        data->state = PLAYER_PKMNS;
        return;
    }
    if (data->playerItemQueued)
    {
        data->state = PLAYER_ITEMS;
        return;
    }
    if (data->opponentAction == BattleActions::BAG && !data->opponentItemDone)
    {
        data->state = OPPONENT_ITEMS;
        return;
    }
    if (data->pendingPlayerReplace
        || (data->playerPkmn && data->playerPkmn->isKO()))
    {
        data->pendingPlayerReplace = false;
        bool hasNext = false;
        for (auto const& pkmn : Game::instance()->data.player.pkmns)
        {
            if (pkmn && !pkmn->isKO())
            {
                hasNext = true;
                break;
            }
        }
        data->playerPkmn = nullptr;
        data->playerConfusion.clear();
        data->state = hasNext ? PKMNS : END;
        return;
    }
    if (data->playerMove && data->playerPkmn && !data->playerPkmn->isKO() && !data->playerBattler.movedThisTurn)
        data->state = PLAYER_MOVES;
    else
        data->state = WEATHER;
}

void SingleBattleOpponentPkmnsPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (speech)
        speech->draw(fps, rs);
}
