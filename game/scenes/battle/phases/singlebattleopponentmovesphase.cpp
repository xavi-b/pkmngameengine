#include "singlebattleopponentmovesphase.h"

#include "../confusionstatus.h"
#include "../moveresolver.h"
#include "../singlebattlescene.h"
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
    auto result = MoveResolver::resolve(*data, false, data->encounterMove);

    if (result.targetFainted && data->playerPkmn && data->playerPkmn->isKO())
    {
        if (!result.messages.empty())
        {
            showSpeech(result.messages);
            step = Step::GoToPlayerFaint;
            return;
        }
        handlePlayerFaint();
        return;
    }

    if (result.userFainted && data->opponentPkmn && data->opponentPkmn->isKO())
    {
        data->opponentConfusion.clear();
        if (!result.messages.empty())
        {
            showSpeech(result.messages);
            step = Step::GoToExperience;
            return;
        }
        scene.resolveOpponentFaint();
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

void SingleBattleOpponentMovesPhase::update(Inputs const* inputs)
{
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
        auto                     result = tickMajorStatus(data->opponentPkmn, messages);
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
            step = Step::AnnounceMove;
            return;
        }
        case ConfusionTickResult::HurtSelf: {
            std::vector<std::string> texts;
            boost::format            confusedText =
                boost::format(lc::translate("%1% is confused!")) % data->opponentPkmn->getDisplayName();
            texts.push_back(confusedText.str());
            texts.push_back(lc::translate("It hurt itself in its confusion!"));

            data->opponentPkmn->decreaseHP(computeConfusionDamage(data->opponentPkmn));
            if (data->opponentPkmn->isKO())
            {
                showSpeech(texts);
                step = Step::GoToExperience;
                return;
            }

            showSpeech(texts);
            step = Step::Advance;
            return;
        }
        case ConfusionTickResult::Continues: {
            boost::format confusedText =
                boost::format(lc::translate("%1% is confused!")) % data->opponentPkmn->getDisplayName();
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
