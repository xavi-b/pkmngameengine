#include "singlebattleplayermovesphase.h"

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
#include <vector>

SingleBattlePlayerMovesPhase::SingleBattlePlayerMovesPhase(SingleBattleScene&                     scene,
                                                           std::shared_ptr<SingleBattleSceneData> data,
                                                           SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattlePlayerMovesPhase::preupdate()
{
    speech.reset();
    step = Step::MajorStatus;
}

void SingleBattlePlayerMovesPhase::showSpeech(std::vector<std::string> const& texts)
{
    speech = std::make_unique<TextSpeech>(renderer);
    speech->setTexts(texts);
    speech->start();
}

void SingleBattlePlayerMovesPhase::advanceAfterAction()
{
    if (data->playerFirst)
        data->state = OPPONENT_MOVES;
    else
        data->state = WEATHER;
}

void SingleBattlePlayerMovesPhase::resolveOpponentFaint()
{
    data->opponentConfusion.clear();
    awardExperienceForOpponentKo();
}

void SingleBattlePlayerMovesPhase::awardExperienceForOpponentKo()
{
    auto hasExpShare = [](Pkmn::PkmnPtr const& pkmn) {
        if (!pkmn)
            return false;
        auto heldItem = pkmn->getHeldItem();
        if (!heldItem || !heldItem->getDefinition())
            return false;
        return heldItem->getDefinition()->getId() == "EXPSHARE";
    };

    // https://bulbapedia.bulbagarden.net/wiki/Experience#Experience_gain_in_battle
    float  a           = battleExperienceMultiplier();
    size_t b           = data->opponentPkmn->getDefinition()->getBaseExp();
    bool   hasLuckyEgg = data->playerPkmn->getHeldItem()
                      && data->playerPkmn->getHeldItem()->getDefinition()->getId() == "LUCKYEGG";
    float  e           = hasLuckyEgg ? 1.5 : 1.0;
    size_t L           = data->opponentPkmn->getLevel();
    float  t = data->opponentPkmn->getFirstTrainerId() == Game::instance()->data.player.id ? 1.5 : 1.0;

    std::vector<Pkmn::PkmnPtr> partyCopy;
    partyCopy.push_back(data->playerPkmn);
    for (auto const& pkmn : Game::instance()->data.player.pkmns)
    {
        if (pkmn && !pkmn->isKO() && pkmn != data->playerPkmn)
            partyCopy.push_back(pkmn);
    }

    auto isParticipant = [this](Pkmn::PkmnPtr const& pkmn) {
        return data->participatingPlayerPkmns.count(pkmn) > 0;
    };

    size_t participatingAliveCount = 0;
    size_t expShareAliveCount      = 0;
    for (auto const& pkmn : partyCopy)
    {
        if (!pkmn || pkmn->isKO())
            continue;
        if (isParticipant(pkmn))
            ++participatingAliveCount;
        if (hasExpShare(pkmn))
            ++expShareAliveCount;
    }

    bool  anyExpShare = expShareAliveCount > 0;
    float baseExp     = b * L / 7.0f * e * a * t;

    for (auto const& pkmn : partyCopy)
    {
        if (!pkmn || pkmn->isKO())
            continue;

        bool  participated = isParticipant(pkmn);
        bool  expShare     = hasExpShare(pkmn);
        float gainedExp    = 0.0f;

        // S rule (Gen 3): without Exp. Share => participants/S.
        // With Exp. Share => participants get base/(2 * participants),
        // holders get base/(2 * holders).
        if (!anyExpShare)
        {
            if (participated && participatingAliveCount > 0)
                gainedExp = baseExp / participatingAliveCount;
        }
        else
        {
            if (participated && participatingAliveCount > 0)
                gainedExp += baseExp / (2.0f * participatingAliveCount);
            if (expShare && expShareAliveCount > 0)
                gainedExp += baseExp / (2.0f * expShareAliveCount);
        }

        size_t expFromBattle = static_cast<size_t>(gainedExp);
        if (expFromBattle > 0)
            data->expGained[pkmn] += expFromBattle;
    }

    onOpponentPkmnDefeated();
    data->state = EXPERIENCE;
}

void SingleBattlePlayerMovesPhase::handlePlayerFaintFromMove(std::vector<std::string> texts)
{
    data->playerConfusion.clear();
    bool hasNext = false;
    for (auto const& p : Game::instance()->data.player.pkmns)
    {
        if (p && !p->isKO())
        {
            hasNext = true;
            break;
        }
    }

    texts.push_back((boost::format(lc::translate("%1% fainted !")) % data->playerPkmn->getDisplayName()).str());
    clearTrapsOnLeave(*data, true);
    data->playerPkmn = nullptr;
    if (hasNext)
    {
        showSpeech(texts);
        step = Step::GoToPkmns;
    }
    else
    {
        texts.push_back(lc::translate("The battle is over ! You lost !"));
        showSpeech(texts);
        step = Step::GoToEnd;
    }
}

void SingleBattlePlayerMovesPhase::executePlayerMove()
{
    bool wasCharging     = data->playerBattler.charging;
    auto result          = MoveResolver::resolve(*data, true, data->playerMove);
    if (result.hit && data->playerMove && data->playerMove->getDefinition())
    {
        auto const definition = data->playerMove->getDefinition();
        battleUi()->startMoveAnim(true,
                                  result.hitPlayerSide,
                                  definition->getId(),
                                  definition->getType(),
                                  definition->getName(),
                                  wasCharging ? 1 : 0,
                                  static_cast<int>(definition->getPower()),
                                  static_cast<int>(result.damageDealt));
    }
    if (result.statRose)
        battleUi()->startStatRise(!result.statRoseOnTarget);
    if (result.healed)
        battleUi()->startHeal(true);
    pendingMessages      = result.messages;
    pendingPlayerFaint   = result.userFainted && data->playerPkmn && data->playerPkmn->isKO();
    pendingOpponentFaint = result.targetFainted && data->opponentPkmn && data->opponentPkmn->isKO();
    if (pendingPlayerFaint)
        data->playerFaintedThisTurn = true;
    if (pendingOpponentFaint)
        data->opponentFaintedThisTurn = true;
    pendingSwitchUser    = result.shouldSwitchUser;
    pendingSwitchTarget  = result.shouldSwitchTarget;
    step                 = Step::AwaitFaint;
    continueAfterPlayerMove();
}

void SingleBattlePlayerMovesPhase::continueAfterPlayerMove()
{
    if (battleUi()->isPresentationRunning())
        return;

    if (pendingOpponentFaint && !playFaint(false))
        return;
    if (pendingPlayerFaint && !playFaint(true))
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
        awardExperienceForOpponentKo();
        return;
    }

    if (pendingPlayerFaint)
    {
        handlePlayerFaintFromMove(pendingMessages);
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

    if (pendingSwitchUser)
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

    if (pendingSwitchTarget && data->opponentPkmn && !data->opponentPkmn->isKO())
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

void SingleBattlePlayerMovesPhase::update(Inputs const* inputs)
{
    if (!speech && step == Step::MajorStatus && !data->pendingAbilityMessages.empty())
    {
        showSpeech(data->pendingAbilityMessages);
        data->pendingAbilityMessages.clear();
        return;
    }

    if (speech)
    {
        inputs->setHandler(speech->inputId());
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
            awardExperienceForOpponentKo();
            return;
        default:
            break;
        }
    }

    if (!data->playerMove)
    {
        advanceAfterAction();
        return;
    }

    switch (step)
    {
    case Step::MajorStatus: {
        if (!data->playerPkmn)
        {
            advanceAfterAction();
            return;
        }

        std::vector<std::string> messages;
        auto                     result = tickMajorStatus(data->playerPkmn, messages, &data->playerBattler);
        switch (result)
        {
        case MajorStatusActionResult::Prevented:
            if (data->playerPkmn->getStatusCondition() == Pkmn::StatusCondition::SLEEP && data->playerMove
                && canMoveWhileAsleep(data->playerMove->getDefinition()))
            {
                step = Step::Flinch;
                break;
            }
            cancelTwoTurnCharge(*data, true);
            data->playerBattler.flinched = false;
            if (char const* script = battleAnimationStatusScript(data->playerPkmn->getStatusCondition()))
                battleUi()->startBattleAnimationScript(true, true, script);
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
        if (data->playerBattler.flinched)
        {
            data->playerBattler.flinched = false;
            cancelTwoTurnCharge(*data, true);
            showSpeech({(boost::format(lc::translate("%1% flinched and couldn't move!"))
                         % data->playerPkmn->getDisplayName())
                            .str()});
            step = Step::Advance;
            return;
        }
        step = Step::Confusion;
        [[fallthrough]];
    }
    case Step::Confusion: {
        if (!data->playerPkmn)
        {
            advanceAfterAction();
            return;
        }

        auto result = tickConfusion(data->playerConfusion);
        switch (result)
        {
        case ConfusionTickResult::SnappedOut: {
            boost::format snapText =
                boost::format(lc::translate("%1% snapped out of confusion!")) % data->playerPkmn->getDisplayName();
            showSpeech({snapText.str()});
            step = Step::Attraction;
            return;
        }
        case ConfusionTickResult::HurtSelf: {
            std::vector<std::string> texts;
            boost::format            confusedText =
                boost::format(lc::translate("%1% is confused!")) % data->playerPkmn->getDisplayName();
            texts.push_back(confusedText.str());
            texts.push_back(lc::translate("It hurt itself in its confusion!"));

            bool magicGuard = !data->playerBattler.abilitySuppressed && data->playerBattler.abilityId == "MAGICGUARD";
            size_t confusionDamage = magicGuard ? 0 : computeConfusionDamage(data->playerPkmn, &data->playerBattler);
            if (!magicGuard)
                data->playerPkmn->decreaseHP(confusionDamage);
            cancelTwoTurnCharge(*data, true);
            if (!battleUi()->startBattleAnimationScript(true, true, "General_MonHit"))
                battleUi()->startHit(true, {});
            if (data->playerPkmn->isKO())
            {
                pendingMessages = texts;
                step            = Step::AwaitPlayerConfusionFaint;
                return;
            }

            showSpeech(texts);
            step = Step::Advance;
            return;
        }
        case ConfusionTickResult::Continues: {
            boost::format confusedText =
                boost::format(lc::translate("%1% is confused!")) % data->playerPkmn->getDisplayName();
            battleUi()->startBattleAnimationScript(true, true, "Status_Confusion");
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
        if (data->playerBattler.infatuated)
        {
            std::string loved = data->opponentPkmn ? data->opponentPkmn->getDisplayName() : std::string("the foe");
            std::vector<std::string> texts;
            texts.push_back((boost::format(lc::translate("%1% is in love with %2%!"))
                             % data->playerPkmn->getDisplayName() % loved)
                                .str());
            battleUi()->startBattleAnimationScript(true, true, "Status_Infatuation");
            if (Utils::randuint(0, 1) == 0)
            {
                texts.push_back((boost::format(lc::translate("%1% is immobilized by love!"))
                                 % data->playerPkmn->getDisplayName())
                                    .str());
                cancelTwoTurnCharge(*data, true);
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
        if (!data->playerPkmn)
        {
            advanceAfterAction();
            return;
        }

        boost::format moveText = boost::format(lc::translate("%1% uses %2% !")) % data->playerPkmn->getDisplayName()
                               % data->playerMove->getDefinition()->getName();
        showSpeech({moveText.str()});
        step = Step::ExecuteMove;
        return;
    }
    case Step::ExecuteMove:
        executePlayerMove();
        return;
    case Step::AwaitFaint:
        continueAfterPlayerMove();
        return;
    case Step::AwaitPlayerConfusionFaint:
        if (!playFaint(true))
            return;
        handlePlayerFaintFromMove(pendingMessages);
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
        awardExperienceForOpponentKo();
        return;
    }
}

void SingleBattlePlayerMovesPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (speech)
        speech->draw(fps, rs);
}
