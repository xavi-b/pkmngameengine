#include "singlebattleplayermovesphase.h"

#include "../confusionstatus.h"
#include "../moveresolver.h"
#include "../singlebattlescene.h"
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
    auto result = MoveResolver::resolve(*data, true, data->playerMove);

    if (result.userFainted && data->playerPkmn && data->playerPkmn->isKO())
    {
        handlePlayerFaintFromMove(result.messages);
        return;
    }

    if (result.targetFainted && data->opponentPkmn && data->opponentPkmn->isKO())
    {
        data->opponentConfusion.clear();
        if (!result.messages.empty())
        {
            showSpeech(result.messages);
            step = Step::GoToExperience;
            return;
        }
        awardExperienceForOpponentKo();
        return;
    }

    if (result.shouldSwitchUser)
    {
        if (!result.messages.empty())
        {
            showSpeech(result.messages);
            step = Step::GoToPkmns;
            return;
        }
        data->state = PKMNS;
        return;
    }

    if (!result.messages.empty())
    {
        showSpeech(result.messages);
        step = Step::Advance;
        return;
    }

    advanceAfterAction();
}

void SingleBattlePlayerMovesPhase::update(Inputs const* inputs)
{
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
        auto                     result = tickMajorStatus(data->playerPkmn, messages);
        switch (result)
        {
        case MajorStatusActionResult::Prevented:
            showSpeech(messages);
            step = Step::Advance;
            return;
        case MajorStatusActionResult::WokeUp:
        case MajorStatusActionResult::Thawed:
            showSpeech(messages);
            step = Step::Confusion;
            return;
        case MajorStatusActionResult::CanAct:
        default:
            step = Step::Confusion;
            break;
        }
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
            step = Step::AnnounceMove;
            return;
        }
        case ConfusionTickResult::HurtSelf: {
            std::vector<std::string> texts;
            boost::format            confusedText =
                boost::format(lc::translate("%1% is confused!")) % data->playerPkmn->getDisplayName();
            texts.push_back(confusedText.str());
            texts.push_back(lc::translate("It hurt itself in its confusion!"));

            data->playerPkmn->decreaseHP(computeConfusionDamage(data->playerPkmn));
            if (data->playerPkmn->isKO())
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

                boost::format faintText =
                    boost::format(lc::translate("%1% fainted !")) % data->playerPkmn->getDisplayName();
                data->playerPkmn = nullptr;

                if (hasNextPkmn)
                {
                    texts.push_back(faintText.str());
                    showSpeech(texts);
                    step = Step::GoToPkmns;
                }
                else
                {
                    texts.push_back(lc::translate("The battle is over ! You lost !"));
                    showSpeech(texts);
                    step = Step::GoToEnd;
                }
                return;
            }

            showSpeech(texts);
            step = Step::Advance;
            return;
        }
        case ConfusionTickResult::Continues: {
            boost::format confusedText =
                boost::format(lc::translate("%1% is confused!")) % data->playerPkmn->getDisplayName();
            showSpeech({confusedText.str()});
            step = Step::AnnounceMove;
            return;
        }
        case ConfusionTickResult::None:
        default:
            step = Step::AnnounceMove;
            break;
        }
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
    case Step::Advance:
        advanceAfterAction();
        return;
    case Step::GoToPkmns:
        data->state = PKMNS;
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
