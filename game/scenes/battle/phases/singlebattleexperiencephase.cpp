#include "singlebattleexperiencephase.h"

#include "../singlebattlescene.h"
#include "game.h"
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

void SingleBattleExperiencePhase::update(Inputs const* inputs)
{
    // TODO: Split experience animation for each level
    if (false) // Experience animation is running
    {
        return;
    }

    if (experienceSpeech)
    {
        experienceSpeech->update(inputs);
        if (experienceSpeech->shouldClose())
            experienceSpeech.release();
        return;
    }

    if (moveToLearnSpeech)
    {
        moveToLearnSpeech->update(inputs);
        if (moveToLearnSpeech->shouldClose())
            moveToLearnSpeech.release();
        // TODO: Ask for new move
        // TODO: Go to NewMovesScene if ok
        data->shouldGoToNewMovesScene = true;
        return;
    }

    if (endSpeech)
    {
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
                {
                    data->evolutionDef = Game::instance()->data.pkmnDefFor(evolutionId);
                    if (data->evolutionDef)
                        data->shouldBreakToEvolution = true;
                }

                for (auto const& moveToLearn : pkmn->getDefinition()->getMovesToLearn())
                {
                    if (moveToLearn.level == pkmn->getLevel())
                    {
                        data->shouldBreakToNewMove = true;
                        break;
                    }
                }

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

    grantExperience(data->expPkmn, remainingExpFromBattle);

    if (data->shouldBreakToLevelUp)
    {
        boost::format experienceText = boost::format(lc::translate("%1% reached level %2% !"))
                                     % data->expPkmn->getDisplayName() % data->expPkmn->getLevel();
        showSpeech({experienceText.str()});
        data->shouldBreakToLevelUp = false;
        return;
    }

    if (data->shouldBreakToEvolution)
    {
        return;
    }

    if (data->shouldBreakToNewMove)
    {
        for (auto const& moveToLearn : data->expPkmn->getDefinition()->getMovesToLearn())
        {
            if (moveToLearn.level == data->expPkmn->getLevel())
            {
                moveToLearnSpeech           = std::make_unique<TextSpeech>(renderer);
                MoveDef::MoveDefPtr moveDef = Game::instance()->data.moveDefFor(moveToLearn.id);
                if (moveDef)
                {
                    boost::format moveToLearnText =
                        boost::format(lc::translate("New move learned: %1%")) % moveDef->getName();
                    moveToLearnSpeech->setTexts({moveToLearnText.str()});
                    moveToLearnSpeech->start();
                    break;
                }
            }
        }
        data->shouldBreakToNewMove = false;
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
