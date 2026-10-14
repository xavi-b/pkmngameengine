#include "singlebattleexperiencephase.h"

#include "../singlebattlescene.h"
#include "game.h"
#include "scenes/newmovesscene.h"
#include "textspeech.h"
#include "utils.h"

#include <boost/format.hpp>

SingleBattleExperiencePhase::SingleBattleExperiencePhase(SingleBattleScene&                     scene,
                                                         std::shared_ptr<SingleBattleSceneData> data,
                                                         SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleExperiencePhase::showSpeech(std::vector<std::string> const& texts)
{
    experienceSpeech = std::make_unique<TextSpeech>(renderer);
    experienceSpeech->setTexts(texts);
    experienceSpeech->start();
}

void SingleBattleExperiencePhase::showEndSpeech(std::string const& text)
{
    endSpeech = std::make_unique<TextSpeech>(renderer);
    endSpeech->setTexts({text});
    endSpeech->start();
}

void SingleBattleExperiencePhase::resumeAfterLevelMessage()
{
    if (!data->pendingMoveIds.empty())
        data->shouldBreakToNewMove = true;
    else if (data->evolutionDef)
        data->shouldBreakToEvolution = true;
}

void SingleBattleExperiencePhase::presentNextMove()
{
    data->shouldBreakToNewMove = false;
    if (!data->expPkmn)
    {
        resumeAfterLevelMessage();
        return;
    }

    while (!data->pendingMoveIds.empty())
    {
        std::string id = data->pendingMoveIds.front();
        data->pendingMoveIds.erase(data->pendingMoveIds.begin());
        data->offeredMoveIds.insert(id);

        MoveDef::MoveDefPtr def = Game::instance()->data.moveDefFor(id);
        if (!def || MoveLearn::knowsMove(data->expPkmn, id))
            continue;

        int slot = MoveLearn::firstEmptySlot(data->expPkmn);
        if (slot >= 0)
        {
            MoveLearn::teachMove(data->expPkmn, def, slot);
            moveToLearnSpeech = std::make_unique<TextSpeech>(renderer);
            boost::format learned =
                boost::format(lc::translate("%1% learned %2% !")) % data->expPkmn->getDisplayName() % def->getName();
            moveToLearnSpeech->setTexts({learned.str()});
            moveToLearnSpeech->start();
            return;
        }

        data->moveToLearnId           = id;
        data->shouldGoToNewMovesScene = true;
        return;
    }

    resumeAfterLevelMessage();
}

void SingleBattleExperiencePhase::update(Inputs const* inputs)
{
    // TODO: Split experience animation for each level
    if (false) // Experience animation is running
    {
        return;
    }

    if (experienceSpeech)
    {
        inputs->setHandler(experienceSpeech->inputId());
        experienceSpeech->update(inputs);
        if (experienceSpeech->shouldClose())
        {
            experienceSpeech.release();
            // Level-up line is read first. Moves are offered before evolution.
            resumeAfterLevelMessage();
        }
        return;
    }

    if (moveToLearnSpeech)
    {
        inputs->setHandler(moveToLearnSpeech->inputId());
        moveToLearnSpeech->update(inputs);
        if (moveToLearnSpeech->shouldClose())
        {
            moveToLearnSpeech.release();
            resumeAfterLevelMessage();
        }
        return;
    }

    if (endSpeech)
    {
        inputs->setHandler(endSpeech->inputId());
        endSpeech->update(inputs);
        if (endSpeech->shouldClose())
        {
            endSpeech.release();
            data->state = END;
        }
        return;
    }

    if (data->expGained.empty())
    {
        if (onExperienceResolvedNextPkmn())
            return;

        showEndSpeech(lc::translate("The battle is over ! You won !"));
        return;
    }

    auto grantExperience = [this](Pkmn::PkmnPtr& pkmn, size_t& expFromBattle) {
        while (expFromBattle > 0)
        {
            size_t expLeftToNextLevel = pkmn->expToNextLevel() - pkmn->getExp();
            if (expFromBattle > expLeftToNextLevel)
            {
                pkmn->incrementLevel();
                std::string evolutionId = SingleBattleScene::canEvolve(pkmn);
                if (!evolutionId.empty())
                    data->evolutionDef = Game::instance()->data.pkmnDefFor(evolutionId);
                else
                    data->evolutionDef.reset();

                data->offeredMoveIds.clear();
                data->pendingMoveIds.clear();
                MoveLearn::queueMoveIds(pkmn, data->pendingMoveIds, data->offeredMoveIds);

                expFromBattle -= expLeftToNextLevel;

                data->shouldBreakToLevelUp = true;

                return;
            }
            else
            {
                pkmn->increaseExp(expFromBattle);
                expFromBattle = 0;
            }
        }
    };

    data->expPkmn                  = data->expGained.begin()->first;
    size_t& remainingExpFromBattle = data->expGained.begin()->second;

    if (data->expFromBattle == 0)
    {
        data->expFromBattle = remainingExpFromBattle;
        boost::format experienceText =
            boost::format(lc::translate("%1% gained %2% XP !")) % data->expPkmn->getDisplayName() % data->expFromBattle;
        showSpeech({experienceText.str()});
        return;
    }

    if (remainingExpFromBattle == 0)
    {
        data->expGained.erase(data->expPkmn);
        data->expPkmn       = nullptr;
        data->expFromBattle = 0;
        return;
    }

    if (data->shouldBreakToEvolution)
        return;

    if (data->shouldBreakToNewMove)
    {
        presentNextMove();
        return;
    }

    grantExperience(data->expPkmn, remainingExpFromBattle);

    if (data->shouldBreakToLevelUp)
    {
        boost::format experienceText = boost::format(lc::translate("%1% reached level %2% !"))
                                     % data->expPkmn->getDisplayName() % data->expPkmn->getLevel();
        showSpeech({experienceText.str()});
        data->shouldBreakToLevelUp = false;
        return;
    }
}

void SingleBattleExperiencePhase::draw(Fps const* fps, RenderSizes rs)
{
    // TODO: Experience animation

    if (experienceSpeech)
    {
        experienceSpeech->draw(fps, rs);
        return;
    }

    if (moveToLearnSpeech)
    {
        moveToLearnSpeech->draw(fps, rs);
        return;
    }

    if (endSpeech)
    {
        endSpeech->draw(fps, rs);
        return;
    }
}
