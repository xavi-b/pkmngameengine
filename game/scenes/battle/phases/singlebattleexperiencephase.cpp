#include "singlebattleexperiencephase.h"

#include "../singlebattlescene.h"
#include "game.h"
#include "pkmnutils.h"
#include "scenes/newmovesscene.h"
#include "textspeech.h"
#include "utils.h"

#include <algorithm>
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

void SingleBattleExperiencePhase::showLevelUpSpeech()
{
    if (!data->expPkmn)
        return;

    boost::format experienceText =
        boost::format(lc::translate("%1% reached level %2% !")) % data->expPkmn->getDisplayName() % data->expPkmn->getLevel();
    showSpeech({experienceText.str()});
    data->shouldBreakToLevelUp = false;
}

void SingleBattleExperiencePhase::showExpOnBar()
{
    if (!data->expPkmn || data->expPkmn != data->playerPkmn)
    {
        battleUi()->clearDisplayedExp();
        return;
    }

    if (expBarCommitted && expBarLevels)
        battleUi()->setDisplayedExp(expBarSpan, expBarSpan);
    else
        battleUi()->setDisplayedExp(expBarShown, expBarSpan);
}

void SingleBattleExperiencePhase::commitExpFill()
{
    if (expBarCommitted || !data->expPkmn || data->expGained.empty())
        return;

    expBarCommitted = true;
    auto& remaining = data->expGained.begin()->second;
    if (remaining >= expBarConsumed)
        remaining -= expBarConsumed;
    else
        remaining = 0;

    if (!expBarLevels)
    {
        data->expPkmn->increaseExp(expBarConsumed);
        return;
    }

    data->expPkmn->incrementLevel();
    std::string evolutionId = PkmnUtils::canEvolve(data->expPkmn);
    if (!evolutionId.empty())
        data->evolutionDef = Game::instance()->data.pkmnDefFor(evolutionId);
    else
        data->evolutionDef.reset();

    data->offeredMoveIds.clear();
    data->pendingMoveIds.clear();
    MoveLearn::queueMoveIds(data->expPkmn, data->pendingMoveIds, data->offeredMoveIds);
    data->shouldBreakToLevelUp = true;
}

void SingleBattleExperiencePhase::beginExpFill()
{
    expBarRunning   = false;
    expBarCommitted = false;
    expBarLevels    = false;
    expBarHold      = 0;
    expBarConsumed  = 0;

    if (!data->expPkmn || data->expGained.empty())
        return;

    size_t& remaining = data->expGained.begin()->second;
    if (remaining == 0)
        return;

    constexpr size_t MaxLevel = 100;
    if (data->expPkmn->getLevel() >= MaxLevel)
    {
        remaining = 0;
        return;
    }

    size_t span    = data->expPkmn->expToNextLevel();
    size_t current = data->expPkmn->getExp();
    if (span == 0)
        span = 1;
    if (current > span)
        current = span;

    size_t room = span - current;
    expBarSpan  = span;

    if (room == 0)
    {
        expBarLevels   = true;
        expBarConsumed = 0;
        commitExpFill();
        return;
    }

    expBarLevels   = remaining >= room;
    expBarConsumed = expBarLevels ? room : remaining;
    expBarTarget   = current + expBarConsumed;
    expBarShown    = current;
    expBarRunning  = true;
    showExpOnBar();
}

bool SingleBattleExperiencePhase::advanceExpFill()
{
    if (!expBarRunning || !data->expPkmn)
    {
        expBarRunning = false;
        battleUi()->clearDisplayedExp();
        return false;
    }

    if (!expBarCommitted)
    {
        size_t span = expBarSpan == 0 ? 1 : expBarSpan;
        size_t step = std::max<size_t>(1, (span + 15) / 16);
        if (expBarShown >= expBarTarget)
            step = 0;
        else if (expBarShown + step > expBarTarget)
            step = expBarTarget - expBarShown;
        expBarShown += step;
        if (expBarShown >= expBarTarget)
            commitExpFill();
        else
        {
            showExpOnBar();
            return true;
        }
    }

    if (expBarCommitted && expBarLevels && expBarHold < 4)
    {
        ++expBarHold;
        showExpOnBar();
        return true;
    }

    battleUi()->clearDisplayedExp();
    expBarRunning = false;
    return false;
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
    if (expBarRunning)
    {
        if (advanceExpFill())
            return;
        if (data->shouldBreakToLevelUp)
        {
            showLevelUpSpeech();
            return;
        }
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

    beginExpFill();
    if (expBarRunning && advanceExpFill())
        return;

    if (data->shouldBreakToLevelUp)
        showLevelUpSpeech();
}

void SingleBattleExperiencePhase::draw(Fps const* fps, RenderSizes rs)
{
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
