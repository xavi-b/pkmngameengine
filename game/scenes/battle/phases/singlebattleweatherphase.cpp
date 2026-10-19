#include "singlebattleweatherphase.h"

#include "../singlebattlescene.h"
#include "../statuscondition.h"
#include "animations/weather/blizzardanimation.h"
#include "animations/weather/harshsunlightanimation.h"
#include "animations/weather/rainanimation.h"
#include "animations/weather/sandstormanimation.h"
#include "animations/weather/weatheranimation.h"
#include "game.h"
#include "itemutils.h"
#include "textspeech.h"
#include "utils.h"

#include <boost/format.hpp>

SingleBattleWeatherPhase::SingleBattleWeatherPhase(SingleBattleScene&                     scene,
                                                   std::shared_ptr<SingleBattleSceneData> data,
                                                   SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleWeatherPhase::setWeather(Map::Weather weather)
{
    data->weather = weather;

    if (weather == Map::Weather::NONE)
    {
        weatherAnimation.release();
        weatherSpeech.release();
        return;
    }

    weatherSpeech = std::make_unique<TextSpeech>(renderer);

    switch (weather)
    {
    case Map::HARSH_SUNLIGHT:
        weatherAnimation = std::make_unique<HarshSunlightAnimation>(renderer, false);
        break;
    case Map::HAIL:
        weatherAnimation = std::make_unique<BlizzardAnimation>(renderer, false);
        break;
    case Map::SANDSTORM:
        weatherAnimation = std::make_unique<SandstormAnimation>(renderer, false);
        break;
    case Map::RAIN:
        weatherAnimation = std::make_unique<RainAnimation>(renderer, false);
        break;
    default:
        break;
    }
    weatherSpeech->setTexts({weatherAnimation->getStartingText()});
    weatherSpeech->start();
}

void SingleBattleWeatherPhase::showSpeech(std::vector<std::string> const& texts)
{
    residualSpeech = std::make_unique<TextSpeech>(renderer);
    residualSpeech->setTexts(texts);
    residualSpeech->start();
}

void SingleBattleWeatherPhase::buildResidualMessages()
{
    residualMessages.clear();
    playerFaintedFromResidual   = false;
    opponentFaintedFromResidual = false;

    auto applyStatusResidual = [this](Pkmn::PkmnPtr const& pkmn, size_t& toxicCounter, bool isPlayer) {
        if (!pkmn || pkmn->isKO())
            return;

        size_t damage = residualStatusDamage(pkmn, toxicCounter);
        if (damage == 0)
            return;

        auto status = pkmn->getStatusCondition();
        pkmn->decreaseHP(damage);

        boost::format text;
        if (status == Pkmn::StatusCondition::BURN)
            text = boost::format(lc::translate("%1% is hurt by its burn!")) % pkmn->getDisplayName();
        else
            text = boost::format(lc::translate("%1% is hurt by poison!")) % pkmn->getDisplayName();
        residualMessages.push_back(text.str());

        if (pkmn->isKO())
        {
            residualMessages.push_back(
                (boost::format(lc::translate("%1% fainted !")) % pkmn->getDisplayName()).str());
            if (isPlayer)
                playerFaintedFromResidual = true;
            else
                opponentFaintedFromResidual = true;
        }
    };

    auto applyBinding = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, bool isPlayer) {
        if (!pkmn || pkmn->isKO() || battler.bindingTurns <= 0)
            return;
        size_t damage = std::max<size_t>(1, pkmn->getTotalHP() / 16);
        pkmn->decreaseHP(damage);
        residualMessages.push_back(
            (boost::format(lc::translate("%1% is hurt by binding!")) % pkmn->getDisplayName()).str());
        --battler.bindingTurns;
        if (battler.bindingTurns <= 0)
        {
            battler.trapped = false;
            residualMessages.push_back(
                (boost::format(lc::translate("%1% was freed from binding!")) % pkmn->getDisplayName()).str());
        }
        if (pkmn->isKO())
        {
            residualMessages.push_back(
                (boost::format(lc::translate("%1% fainted !")) % pkmn->getDisplayName()).str());
            if (isPlayer)
                playerFaintedFromResidual = true;
            else
                opponentFaintedFromResidual = true;
        }
    };

    auto applyLeechSeed = [this](Pkmn::PkmnPtr const& victim, BattleBattler& victimBattler,
                                 Pkmn::PkmnPtr const& recipient, bool victimIsPlayer) {
        if (!victim || victim->isKO() || !victimBattler.seeded || !recipient || recipient->isKO())
            return;
        size_t damage = std::max<size_t>(1, victim->getTotalHP() / 8);
        victim->decreaseHP(damage);
        size_t heal = std::min(damage, recipient->getTotalHP() - recipient->getHP());
        if (heal > 0)
            recipient->setHP(recipient->getHP() + heal);
        residualMessages.push_back(
            (boost::format(lc::translate("%1%'s health is sapped by Leech Seed!")) % victim->getDisplayName()).str());
        if (victim->isKO())
        {
            residualMessages.push_back(
                (boost::format(lc::translate("%1% fainted !")) % victim->getDisplayName()).str());
            if (victimIsPlayer)
                playerFaintedFromResidual = true;
            else
                opponentFaintedFromResidual = true;
        }
    };

    auto applyPerish = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, bool isPlayer) {
        if (!pkmn || pkmn->isKO() || battler.perishCount < 0)
            return;
        residualMessages.push_back((boost::format(lc::translate("%1%'s perish count fell to %2%!"))
                                    % pkmn->getDisplayName() % battler.perishCount)
                                       .str());
        if (battler.perishCount == 0)
        {
            pkmn->setHP(0);
            residualMessages.push_back(
                (boost::format(lc::translate("%1% fainted !")) % pkmn->getDisplayName()).str());
            if (isPlayer)
                playerFaintedFromResidual = true;
            else
                opponentFaintedFromResidual = true;
        }
        else
            --battler.perishCount;
    };

    auto applyYawn = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler) {
        if (!pkmn || pkmn->isKO() || !battler.yawnPending)
            return;
        battler.yawnPending = false;
        size_t dummy        = 0;
        if (applyStatusCondition(pkmn, Pkmn::StatusCondition::SLEEP, dummy))
            residualMessages.push_back(
                (boost::format(lc::translate("%1% fell asleep!")) % pkmn->getDisplayName()).str());
    };

    // End-of-turn field timers
    if (data->trickRoom && data->trickRoomTurns > 0 && --data->trickRoomTurns == 0)
    {
        data->trickRoom = false;
        residualMessages.push_back(lc::translate("The twisted dimensions returned to normal!"));
    }
    if (data->gravity && data->gravityTurns > 0 && --data->gravityTurns == 0)
    {
        data->gravity = false;
        residualMessages.push_back(lc::translate("Gravity returned to normal!"));
    }
    if (data->terrain != SingleBattleSceneData::Terrain::None && data->terrainTurns > 0
        && --data->terrainTurns == 0)
    {
        data->terrain = SingleBattleSceneData::Terrain::None;
        residualMessages.push_back(lc::translate("The terrain returned to normal!"));
    }

    // Clear single-turn protect / flinch / endure
    data->playerBattler.protect  = false;
    data->opponentBattler.protect = false;
    data->playerBattler.endure   = false;
    data->opponentBattler.endure = false;
    data->playerBattler.flinched = false;
    data->opponentBattler.flinched = false;

    bool playerFirst = true;
    if (data->playerPkmn && data->opponentPkmn)
    {
        size_t ps = battleSpeed(data->playerPkmn, &data->playerBattler);
        size_t os = battleSpeed(data->opponentPkmn, &data->opponentBattler);
        if (data->trickRoom)
            std::swap(ps, os);
        playerFirst = ps >= os;
    }

    auto applyHeld = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, ConfusionStatus& confusion,
                            bool isPlayer) {
        if (!pkmn || pkmn->isKO())
            return;
        auto tick = ItemUtils::tickHeldItem(pkmn, &battler, &confusion);
        if (!tick.message.empty())
            residualMessages.push_back(tick.message);
        if (tick.fainted || pkmn->isKO())
        {
            residualMessages.push_back(
                (boost::format(lc::translate("%1% fainted !")) % pkmn->getDisplayName()).str());
            if (isPlayer)
                playerFaintedFromResidual = true;
            else
                opponentFaintedFromResidual = true;
        }
    };

    auto runSide = [&](bool isPlayer) {
        if (isPlayer)
        {
            applyStatusResidual(data->playerPkmn, data->playerBattler.toxicCounter, true);
            if (!playerFaintedFromResidual)
                applyBinding(data->playerPkmn, data->playerBattler, true);
            if (!playerFaintedFromResidual)
                applyLeechSeed(data->playerPkmn, data->playerBattler, data->opponentPkmn, true);
            if (!playerFaintedFromResidual)
                applyPerish(data->playerPkmn, data->playerBattler, true);
            if (!playerFaintedFromResidual)
                applyYawn(data->playerPkmn, data->playerBattler);
            if (!playerFaintedFromResidual)
                applyHeld(data->playerPkmn, data->playerBattler, data->playerConfusion, true);
        }
        else
        {
            applyStatusResidual(data->opponentPkmn, data->opponentBattler.toxicCounter, false);
            if (!opponentFaintedFromResidual)
                applyBinding(data->opponentPkmn, data->opponentBattler, false);
            if (!opponentFaintedFromResidual)
                applyLeechSeed(data->opponentPkmn, data->opponentBattler, data->playerPkmn, false);
            if (!opponentFaintedFromResidual)
                applyPerish(data->opponentPkmn, data->opponentBattler, false);
            if (!opponentFaintedFromResidual)
                applyYawn(data->opponentPkmn, data->opponentBattler);
            if (!opponentFaintedFromResidual)
                applyHeld(data->opponentPkmn, data->opponentBattler, data->opponentConfusion, false);
        }
    };

    if (playerFirst)
    {
        runSide(true);
        if (!playerFaintedFromResidual)
            runSide(false);
    }
    else
    {
        runSide(false);
        if (!opponentFaintedFromResidual)
            runSide(true);
    }
}

void SingleBattleWeatherPhase::finishTurn()
{
    if (opponentFaintedFromResidual && data->opponentPkmn && data->opponentPkmn->isKO())
    {
        data->opponentConfusion.clear();
        scene.resolveOpponentFaint();
        return;
    }

    if (playerFaintedFromResidual)
    {
        bool hasNextPkmn = false;
        for (auto const& pkmn : Game::instance()->data.player.pkmns)
        {
            if (pkmn && !pkmn->isKO())
            {
                hasNextPkmn = true;
                break;
            }
        }
        data->playerPkmn = nullptr;
        data->playerConfusion.clear();
        data->state = hasNextPkmn ? PKMNS : END;
        return;
    }

    data->state = ACTIONS;
}

void SingleBattleWeatherPhase::preupdate()
{
    residualSpeech.release();
    residualMessages.clear();
    residualsApplied            = false;
    playerFaintedFromResidual   = false;
    opponentFaintedFromResidual = false;

    if (data->weather != Map::Weather::NONE && weatherAnimation)
    {
        step = Step::Weather;
        if (weatherSpeech)
        {
            weatherSpeech->setTexts({weatherAnimation->getContinuingText()});
            weatherSpeech->reset();
            weatherSpeech->start();
        }
        weatherAnimation->reset();
    }
    else
    {
        step = Step::Residuals;
    }
}

void SingleBattleWeatherPhase::update(Inputs const* inputs)
{
    if (residualSpeech)
    {
        inputs->setHandler(residualSpeech->inputId());
        residualSpeech->update(inputs);
        if (!residualSpeech->shouldClose())
            return;
        residualSpeech.release();
        finishTurn();
        return;
    }

    if (step == Step::Weather)
    {
        if (data->weather == Map::Weather::NONE || !weatherAnimation)
        {
            step = Step::Residuals;
        }
        else
        {
            if (!weatherAnimation->isStarted())
                weatherAnimation->start();

            if (weatherAnimation->isRunning())
                weatherAnimation->incrementTicks();

            if (weatherSpeech)
            {
                inputs->setHandler(weatherSpeech->inputId());
                weatherSpeech->update(inputs);
            }

            if (weatherSpeech && weatherSpeech->shouldClose())
                step = Step::Residuals;
            else
                return;
        }
    }

    if (step == Step::Residuals)
    {
        if (!residualsApplied)
        {
            buildResidualMessages();
            residualsApplied = true;
        }
        if (opponentFaintedFromResidual && data->opponentPkmn && data->opponentPkmn->isKO() && !playFaint(false))
            return;
        if (playerFaintedFromResidual && data->playerPkmn && data->playerPkmn->isKO() && !playFaint(true))
            return;
        if (!residualMessages.empty())
        {
            showSpeech(residualMessages);
            return;
        }
        finishTurn();
        return;
    }

    finishTurn();
}

void SingleBattleWeatherPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (weatherAnimation && weatherAnimation->isRunning() && step == Step::Weather)
        weatherAnimation->draw(fps, rs);

    if (residualSpeech)
    {
        residualSpeech->draw(fps, rs);
        return;
    }

    if (weatherSpeech && step == Step::Weather && !weatherSpeech->shouldClose())
        weatherSpeech->draw(fps, rs);
}
