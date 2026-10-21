#include "singlebattleopponentmovesphase.h"

#include "../abilitybattle.h"
#include "../confusionstatus.h"
#include "../moveeffects.h"
#include "../moveresolver.h"
#include "../singlebattlescene.h"
#include "../trainerscenedata.h"
#include "../statuscondition.h"
#include "game.h"
#include "textspeech.h"
#include "utils.h"

#include <boost/format.hpp>
#include <iostream>
#include <vector>

SingleBattleOpponentMovesPhase::SingleBattleOpponentMovesPhase(SingleBattleScene&                     scene,
                                                               std::shared_ptr<SingleBattleSceneData> data,
                                                               SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleOpponentMovesPhase::preupdate()
{
    speech.reset();
    step = Step::MajorStatus;
}

void SingleBattleOpponentMovesPhase::showSpeech(std::vector<std::string> const& texts)
{
    speech = std::make_unique<TextSpeech>(renderer);
    speech->setTexts(texts);
    speech->start();
}

void SingleBattleOpponentMovesPhase::advanceAfterAction()
{
    if (data->playerFirst)
        data->state = WEATHER;
    else
        data->state = PLAYER_MOVES;
}

void SingleBattleOpponentMovesPhase::handlePlayerFaint()
{
    data->playerConfusion.clear();

    bool hasNextPkmn = false;
    for (auto const& pkmn : Game::instance()->data.player.pkmns)
    {
        if (pkmn && !pkmn->isKO())
        {
            hasNextPkmn = true;
            break;
        }
    }

    boost::format faintText = boost::format(lc::translate("%1% fainted !")) % data->playerPkmn->getDisplayName();
    clearTrapsOnLeave(*data, true);
    data->playerPkmn        = nullptr;

    if (hasNextPkmn)
    {
        showSpeech({faintText.str()});
        step = Step::GoToPkmns;
    }
    else
    {
        showSpeech({faintText.str(), lc::translate("The battle is over ! You lost !")});
        step = Step::GoToEnd;
    }
}

void SingleBattleOpponentMovesPhase::executeOpponentMove()
{
    bool wasCharging     = data->opponentBattler.charging;
    auto result          = MoveResolver::resolve(*data, false, data->encounterMove);
    if (result.hit && data->encounterMove && data->encounterMove->getDefinition())
    {
        auto const definition = data->encounterMove->getDefinition();
        battleUi()->startMoveAnim(false,
                                  result.hitPlayerSide,
                                  definition->getId(),
                                  definition->getType(),
                                  definition->getName(),
                                  wasCharging ? 1 : 0,
                                  static_cast<int>(definition->getPower()),
                                  static_cast<int>(result.damageDealt));
    }
    if (result.statRose)
        battleUi()->startStatRise(result.statRoseOnTarget);
    if (result.healed)
        battleUi()->startHeal(false);
    pendingMessages      = result.messages;
    pendingPlayerFaint   = result.targetFainted && data->playerPkmn && data->playerPkmn->isKO();
    pendingOpponentFaint = result.userFainted && data->opponentPkmn && data->opponentPkmn->isKO();
    if (pendingPlayerFaint)
        data->playerFaintedThisTurn = true;
    if (pendingOpponentFaint)
        data->opponentFaintedThisTurn = true;
    pendingSwitchUser    = result.shouldSwitchUser;
    pendingSwitchTarget  = result.shouldSwitchTarget;
    step                 = Step::AwaitFaint;
    continueAfterOpponentMove();
}

void SingleBattleOpponentMovesPhase::continueAfterOpponentMove()
{
    if (battleUi()->isPresentationRunning())
        return;

    if (pendingPlayerFaint && !playFaint(true))
        return;
    if (pendingOpponentFaint && !playFaint(false))
        return;

    if (pendingOpponentFaint)
    {
        data->opponentConfusion.clear();
        clearTrapsOnLeave(*data, false);
        if (pendingPlayerFaint)
            data->pendingPlayerReplace = true;
        if (!pendingMessages.empty())
        {
            showSpeech(pendingMessages);
            step = Step::GoToExperience;
            return;
        }
        scene.resolveOpponentFaint();
        return;
    }

    if (pendingPlayerFaint)
    {
        if (!pendingMessages.empty())
        {
            showSpeech(pendingMessages);
            step = Step::GoToPlayerFaint;
            return;
        }
        handlePlayerFaint();
        return;
    }

    if (data->forceWildEnd)
    {
        data->forceWildEnd = false;
        if (!pendingMessages.empty())
        {
            showSpeech(pendingMessages);
            step = Step::GoToOpponentRun;
            return;
        }
        data->state = OPPONENT_RUN;
        return;
    }

    if (pendingSwitchTarget && data->playerPkmn && !data->playerPkmn->isKO())
    {
        if (!pendingMessages.empty())
        {
            showSpeech(pendingMessages);
            step = Step::GoToPkmns;
            return;
        }
        data->state = PKMNS;
        return;
    }

    if (pendingSwitchUser && data->opponentPkmn && !data->opponentPkmn->isKO())
    {
        auto trainerData = std::dynamic_pointer_cast<TrainerSceneData>(data);
        if (trainerData && trainerData->opponentTrainer)
        {
            std::vector<Pkmn::PkmnPtr> candidates;
            for (auto const& pkmn : trainerData->opponentTrainer->pkmns)
            {
                if (pkmn && !pkmn->isKO() && pkmn != data->opponentPkmn)
                    candidates.push_back(pkmn);
            }
            if (!candidates.empty())
            {
                trainerData->nextOpponentPkmn = candidates[Utils::randuint(0, candidates.size() - 1)];
                if (!pendingMessages.empty())
                {
                    showSpeech(pendingMessages);
                    step = Step::GoToOpponentPkmns;
                    return;
                }
                data->state = OPPONENT_PKMNS;
                return;
            }
        }
    }

    if (!pendingMessages.empty())
    {
        showSpeech(pendingMessages);
        step = Step::Advance;
        return;
    }

    advanceAfterAction();
}

void SingleBattleOpponentMovesPhase::update(Inputs const* inputs)
{
    if (!speech && step == Step::MajorStatus && !data->pendingAbilityMessages.empty())
    {
        showSpeech(data->pendingAbilityMessages);
        data->pendingAbilityMessages.clear();
        return;
    }

    if (speech)
    {
        speech->update(inputs);
        if (!speech->shouldClose())
            return;
        speech.release();

        switch (step)
        {
        case Step::Advance:
            advanceAfterAction();
            return;
        case Step::GoToPkmns:
            data->state = PKMNS;
            return;
        case Step::GoToOpponentPkmns:
            data->state = OPPONENT_PKMNS;
            return;
        case Step::GoToOpponentRun:
            data->state = OPPONENT_RUN;
            return;
        case Step::GoToEnd:
            data->state = END;
            return;
        case Step::GoToExperience:
            scene.resolveOpponentFaint();
            return;
        case Step::GoToPlayerFaint:
            handlePlayerFaint();
            return;
        default:
            break;
        }
    }

    if (!data->encounterMove || data->encounterMove->getCurrentPP() == 0)
    {
        advanceAfterAction();
        return;
    }

    if (Game::instance()->isDebug())
        std::cout << __PRETTY_FUNCTION__ << " Move: " << data->encounterMove->getDefinition()->getId() << std::endl;

    switch (step)
    {
    case Step::MajorStatus: {
        if (!data->opponentPkmn)
        {
            advanceAfterAction();
            return;
        }

        std::vector<std::string> messages;
        auto                     result = tickMajorStatus(data->opponentPkmn, messages, &data->opponentBattler);
        switch (result)
        {
        case MajorStatusActionResult::Prevented:
            if (data->opponentPkmn->getStatusCondition() == Pkmn::StatusCondition::SLEEP && data->encounterMove
                && canMoveWhileAsleep(data->encounterMove->getDefinition()))
            {
                step = Step::Flinch;
                break;
            }
            cancelTwoTurnCharge(*data, false);
            data->opponentBattler.flinched = false;
            if (char const* script = battleAnimationStatusScript(data->opponentPkmn->getStatusCondition()))
                battleUi()->startBattleAnimationScript(false, false, script);
            showSpeech(messages);
            step = Step::Advance;
            return;
        case MajorStatusActionResult::WokeUp:
        case MajorStatusActionResult::Thawed:
            showSpeech(messages);
            step = Step::Flinch;
            return;
        case MajorStatusActionResult::CanAct:
        default:
            step = Step::Flinch;
            break;
        }
        [[fallthrough]];
    }
    case Step::Flinch: {
        if (data->opponentBattler.flinched)
        {
            data->opponentBattler.flinched = false;
            cancelTwoTurnCharge(*data, false);
            showSpeech({(boost::format(lc::translate("%1% flinched and couldn't move!"))
                         % data->opponentPkmn->getDisplayName())
                            .str()});
            step = Step::Advance;
            return;
        }
        step = Step::Confusion;
        [[fallthrough]];
    }
    case Step::Confusion: {
        if (!data->opponentPkmn)
        {
            advanceAfterAction();
            return;
        }

        auto result = tickConfusion(data->opponentConfusion);
        switch (result)
        {
        case ConfusionTickResult::SnappedOut: {
            boost::format snapText =
                boost::format(lc::translate("%1% snapped out of confusion!")) % data->opponentPkmn->getDisplayName();
            showSpeech({snapText.str()});
            step = Step::Attraction;
            return;
        }
        case ConfusionTickResult::HurtSelf: {
            std::vector<std::string> texts;
            boost::format            confusedText =
                boost::format(lc::translate("%1% is confused!")) % data->opponentPkmn->getDisplayName();
            texts.push_back(confusedText.str());
            texts.push_back(lc::translate("It hurt itself in its confusion!"));

            bool magicGuard = !data->opponentBattler.abilitySuppressed && data->opponentBattler.abilityId == "MAGICGUARD";
            size_t confusionDamage = magicGuard ? 0 : computeConfusionDamage(data->opponentPkmn, &data->opponentBattler);
            if (!magicGuard)
                data->opponentPkmn->decreaseHP(confusionDamage);
            cancelTwoTurnCharge(*data, false);
            if (!battleUi()->startBattleAnimationScript(false, false, "General_MonHit"))
                battleUi()->startHit(false, {});
            if (data->opponentPkmn->isKO())
            {
                pendingMessages = texts;
                step            = Step::AwaitOpponentConfusionFaint;
                return;
            }

            showSpeech(texts);
            step = Step::Advance;
            return;
        }
        case ConfusionTickResult::Continues: {
            boost::format confusedText =
                boost::format(lc::translate("%1% is confused!")) % data->opponentPkmn->getDisplayName();
            battleUi()->startBattleAnimationScript(false, false, "Status_Confusion");
            showSpeech({confusedText.str()});
            step = Step::Attraction;
            return;
        }
        case ConfusionTickResult::None:
        default:
            step = Step::Attraction;
            break;
        }
        [[fallthrough]];
    }
    case Step::Attraction: {
        if (data->opponentBattler.infatuated)
        {
            std::string loved = data->playerPkmn ? data->playerPkmn->getDisplayName() : std::string("the foe");
            std::vector<std::string> texts;
            texts.push_back((boost::format(lc::translate("%1% is in love with %2%!"))
                             % data->opponentPkmn->getDisplayName() % loved)
                                .str());
            battleUi()->startBattleAnimationScript(false, false, "Status_Infatuation");
            if (Utils::randuint(0, 1) == 0)
            {
                texts.push_back((boost::format(lc::translate("%1% is immobilized by love!"))
                                 % data->opponentPkmn->getDisplayName())
                                    .str());
                cancelTwoTurnCharge(*data, false);
                showSpeech(texts);
                step = Step::Advance;
                return;
            }
            showSpeech(texts);
            step = Step::AnnounceMove;
            return;
        }
        step = Step::AnnounceMove;
        [[fallthrough]];
    }
    case Step::AnnounceMove: {
        if (!data->encounterMove)
        {
            advanceAfterAction();
            return;
        }

        showSpeech({opponentMoveText(data->encounterMove)});
        step = Step::ExecuteMove;
        return;
    }
    case Step::ExecuteMove:
        executeOpponentMove();
        return;
    case Step::AwaitFaint:
        continueAfterOpponentMove();
        return;
    case Step::AwaitOpponentConfusionFaint:
        if (!playFaint(false))
            return;
        showSpeech(pendingMessages);
        step = Step::GoToExperience;
        return;
    case Step::Advance:
        advanceAfterAction();
        return;
    case Step::GoToPkmns:
        data->state = PKMNS;
        return;
    case Step::GoToOpponentPkmns:
        data->state = OPPONENT_PKMNS;
        return;
    case Step::GoToOpponentRun:
        data->state = OPPONENT_RUN;
        return;
    case Step::GoToEnd:
        data->state = END;
        return;
    case Step::GoToExperience:
        scene.resolveOpponentFaint();
        return;
    case Step::GoToPlayerFaint:
        handlePlayerFaint();
        return;
    }
}

void SingleBattleOpponentMovesPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (speech)
        speech->draw(fps, rs);
}
