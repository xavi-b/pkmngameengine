#include "singlebattleweatherphase.h"

#include "../abilitybattle.h"
#include "../confusionstatus.h"
#include "../moveeffects.h"
#include "../singlebattlescene.h"
#include "../statuscondition.h"
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
    scene.applyFieldWeather(weather);

    if (weather == Map::Weather::NONE || !scene.fieldWeatherAnim())
    {
        weatherSpeech.reset();
        return;
    }

    weatherSpeech = std::make_unique<TextSpeech>(renderer);
    weatherSpeech->setTexts({scene.fieldWeatherAnim()->getStartingText()});
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
    if (!data->pendingAbilityMessages.empty())
    {
        residualMessages.insert(residualMessages.end(), data->pendingAbilityMessages.begin(),
                                data->pendingAbilityMessages.end());
        data->pendingAbilityMessages.clear();
    }
    playerFaintedFromResidual   = false;
    opponentFaintedFromResidual = false;

    auto applyStatusResidual = [this](Pkmn::PkmnPtr const& pkmn, size_t& toxicCounter, bool isPlayer) {
        if (!pkmn || pkmn->isKO())
            return;

        BattleBattler* battler = isPlayer ? &data->playerBattler : &data->opponentBattler;
        if (battler && !battler->abilitySuppressed && battler->abilityId == "MAGICGUARD")
            return;
        if (battler && !battler->abilitySuppressed && battler->abilityId == "POISONHEAL"
            && (pkmn->getStatusCondition() == Pkmn::StatusCondition::POISON
                || pkmn->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON))
        {
            if (battler->healBlockTurns > 0)
                return;
            size_t heal = std::max<size_t>(1, pkmn->getTotalHP() / 8);
            pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + heal));
            residualMessages.push_back(
                (boost::format(lc::translate("%1%'s Poison Heal restored its HP!")) % pkmn->getDisplayName()).str());
            return;
        }
        size_t damage = residualStatusDamage(pkmn, toxicCounter, battler);
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
        if (char const* script = battleAnimationStatusScript(status))
            battleUi()->startBattleAnimationScript(isPlayer, isPlayer, script);

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
        if (!battler.abilitySuppressed && battler.abilityId == "MAGICGUARD")
        {
            --battler.bindingTurns;
            if (battler.bindingTurns <= 0)
            {
                if (!battler.meanLooked && !battler.octolock && !battler.jawLocked && !battler.ingrain)
                    battler.trapped = false;
                residualMessages.push_back(
                    (boost::format(lc::translate("%1% was freed from binding!")) % pkmn->getDisplayName()).str());
            }
            return;
        }
        size_t damage = std::max<size_t>(1, pkmn->getTotalHP() / 16);
        pkmn->decreaseHP(damage);
        residualMessages.push_back(
            (boost::format(lc::translate("%1% is hurt by binding!")) % pkmn->getDisplayName()).str());
        battleUi()->startBattleAnimationScript(isPlayer, isPlayer, "Status_BindWrap");
        --battler.bindingTurns;
        if (battler.bindingTurns <= 0)
        {
            if (!battler.meanLooked && !battler.octolock && !battler.jawLocked && !battler.ingrain)
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
        if (!victimBattler.abilitySuppressed && victimBattler.abilityId == "MAGICGUARD")
            return;
        size_t damage = std::max<size_t>(1, victim->getTotalHP() / 8);
        BattleBattler& recipientBattler = victimIsPlayer ? data->opponentBattler : data->playerBattler;
        if (ItemUtils::activeHeldId(recipient, &recipientBattler) == "BIGROOT")
            damage = std::max<size_t>(1, damage * 13 / 10);
        victim->decreaseHP(damage);
        bool liquidOoze = !recipientBattler.abilitySuppressed && recipientBattler.abilityId == "LIQUIDOOZE";
        bool recipientGuard = !recipientBattler.abilitySuppressed && recipientBattler.abilityId == "MAGICGUARD";
        if (liquidOoze)
        {
            if (!recipientGuard)
            {
                recipient->decreaseHP(damage);
                residualMessages.push_back((boost::format(lc::translate("%1% was hurt by %2%'s Liquid Ooze!"))
                                            % recipient->getDisplayName() % recipient->getDisplayName())
                                               .str());
                if (recipient->isKO())
                {
                    residualMessages.push_back(
                        (boost::format(lc::translate("%1% fainted !")) % recipient->getDisplayName()).str());
                    if (victimIsPlayer)
                        opponentFaintedFromResidual = true;
                    else
                        playerFaintedFromResidual = true;
                }
            }
        }
        else if (recipientBattler.healBlockTurns <= 0)
        {
            size_t heal = std::min(damage, recipient->getTotalHP() - recipient->getHP());
            if (heal > 0)
                recipient->setHP(recipient->getHP() + heal);
        }
        residualMessages.push_back(
            (boost::format(lc::translate("%1%'s health is sapped by Leech Seed!")) % victim->getDisplayName()).str());
        battleUi()->startBattleAnimationScript(!victimIsPlayer, victimIsPlayer, "General_LeechSeedDrain");
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

    auto applyCurse = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, bool isPlayer) {
        if (!pkmn || pkmn->isKO() || !battler.cursed)
            return;
        if (!battler.abilitySuppressed && battler.abilityId == "MAGICGUARD")
            return;
        size_t damage = std::max<size_t>(1, pkmn->getTotalHP() / 4);
        pkmn->decreaseHP(damage);
        residualMessages.push_back(
            (boost::format(lc::translate("%1% is hurt by the curse!")) % pkmn->getDisplayName()).str());
        battleUi()->startBattleAnimationScript(isPlayer, isPlayer, "Status_Curse");
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

    auto applyPerish = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, bool isPlayer) {
        if (!pkmn || pkmn->isKO() || battler.perishCount < 0)
            return;
        if (battler.perishCount > 0)
            --battler.perishCount;
        residualMessages.push_back((boost::format(lc::translate("%1%'s perish count fell to %2%!"))
                                    % pkmn->getDisplayName() % battler.perishCount)
                                       .str());
        if (battler.perishCount <= 0)
        {
            battler.perishCount = -1;
            pkmn->setHP(0);
            residualMessages.push_back(
                (boost::format(lc::translate("%1% fainted !")) % pkmn->getDisplayName()).str());
            if (isPlayer)
                playerFaintedFromResidual = true;
            else
                opponentFaintedFromResidual = true;
        }
    };

    auto applyWish = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, int& turns, size_t& heal) {
        if (turns <= 0)
            return;
        if (turns > 1)
        {
            --turns;
            return;
        }
        turns = 0;
        size_t amount = std::max<size_t>(1, heal);
        heal = 0;
        if (!pkmn || pkmn->isKO() || pkmn->getHP() >= pkmn->getTotalHP() || battler.healBlockTurns > 0)
            return;
        pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + amount));
        residualMessages.push_back(
            (boost::format(lc::translate("%1%'s wish came true!")) % pkmn->getDisplayName()).str());
        bool wishPlayer = &battler == &data->playerBattler;
        battleUi()->startBattleAnimationScript(wishPlayer, wishPlayer, "General_WishHeal");
    };

    auto applyYawn = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler) {
        if (!pkmn || pkmn->isKO() || battler.yawnTurns <= 0)
            return;
        if (battler.yawnTurns > 1)
        {
            --battler.yawnTurns;
            return;
        }
        battler.yawnTurns = 0;
        size_t dummy      = 0;
        if (data->playerBattler.uproarTurns > 0 || data->opponentBattler.uproarTurns > 0)
        {
            residualMessages.push_back(
                (boost::format(lc::translate("%1% stayed awake in the uproar!")) % pkmn->getDisplayName()).str());
            return;
        }
        int safeguard = &battler == &data->playerBattler ? data->playerSafeguard : data->opponentSafeguard;
        if (safeguard > 0)
        {
            residualMessages.push_back(
                (boost::format(lc::translate("%1% is protected by Safeguard!")) % pkmn->getDisplayName()).str());
            return;
        }
        if (!battler.abilitySuppressed && battler.abilityId == "LEAFGUARD"
            && data->fieldWeather() == Map::Weather::HARSH_SUNLIGHT
            && ItemUtils::activeHeldId(pkmn, &battler) != "UTILITYUMBRELLA")
            return;
        if (abilityBlocksStatus(pkmn, battler, Pkmn::StatusCondition::SLEEP))
            return;
        if (applyStatusCondition(pkmn, Pkmn::StatusCondition::SLEEP, dummy, false, &battler))
            residualMessages.push_back(
                (boost::format(lc::translate("%1% fell asleep!")) % pkmn->getDisplayName()).str());
    };

    bool weatherEnding = false;
    if (data->weatherFromMove && data->weatherTurns > 0 && --data->weatherTurns == 0)
        weatherEnding = true;

    auto tickScreen = [this](int& turns, char const* woreOff) {
        if (turns > 0 && --turns == 0)
            residualMessages.push_back(lc::translate(woreOff));
    };
    tickScreen(data->playerReflect, "Your team's Reflect wore off!");
    tickScreen(data->opponentReflect, "The opposing team's Reflect wore off!");
    tickScreen(data->playerLightScreen, "Your team's Light Screen wore off!");
    tickScreen(data->opponentLightScreen, "The opposing team's Light Screen wore off!");
    tickScreen(data->playerAuroraVeil, "Your team's Aurora Veil wore off!");
    tickScreen(data->opponentAuroraVeil, "The opposing team's Aurora Veil wore off!");
    tickScreen(data->playerSafeguard, "Your team's Safeguard wore off!");
    tickScreen(data->opponentSafeguard, "The opposing team's Safeguard wore off!");
    tickScreen(data->playerMist, "Your team's mist faded!");
    tickScreen(data->opponentMist, "The opposing team's mist faded!");
    tickScreen(data->playerLuckyChant, "Your team's Lucky Chant wore off!");
    tickScreen(data->opponentLuckyChant, "The opposing team's Lucky Chant wore off!");
    tickScreen(data->mudSportTurns, "Mud Sport wore off!");
    tickScreen(data->waterSportTurns, "Water Sport wore off!");
    if (data->fairyLockTurns > 0 && --data->fairyLockTurns == 0)
        data->fairyLock = false;

    auto weatherChip = [this](Pkmn::PkmnPtr const& pkmn, bool isPlayer) {
        if (!pkmn || pkmn->isKO())
            return;
        bool sand = data->fieldWeather() == Map::SANDSTORM;
        bool hail = data->fieldWeather() == Map::HAIL;
        if (!sand && !hail)
            return;
        BattleBattler const& battler = isPlayer ? data->playerBattler : data->opponentBattler;
        if (!battler.abilitySuppressed && (battler.abilityId == "MAGICGUARD" || battler.abilityId == "OVERCOAT"))
            return;
        if (ItemUtils::activeHeldId(pkmn, &battler) == "SAFETYGOGGLES")
            return;
        if (!battler.abilitySuppressed && sand
            && (battler.abilityId == "SANDVEIL" || battler.abilityId == "SANDRUSH" || battler.abilityId == "SANDFORCE"))
            return;
        if (!battler.abilitySuppressed && hail && (battler.abilityId == "ICEBODY" || battler.abilityId == "SNOWCLOAK"))
            return;
        if (sand && (hasType(pkmn, "ROCK", &battler) || hasType(pkmn, "GROUND", &battler) || hasType(pkmn, "STEEL", &battler)))
            return;
        if (hail && hasType(pkmn, "ICE", &battler))
            return;
        pkmn->decreaseHP(std::max<size_t>(1, pkmn->getTotalHP() / 16));
        residualMessages.push_back(
            (boost::format(lc::translate(sand ? "%1% is buffeted by the sandstorm!" : "%1% is buffeted by the hail!"))
             % pkmn->getDisplayName())
                .str());
        battleUi()->startBattleAnimationScript(isPlayer, isPlayer, sand ? "General_Sandstorm" : "General_Hail");
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
    weatherChip(data->opponentPkmn, false);
    weatherChip(data->playerPkmn, true);

    auto fireDelayed = [this](DelayedStrike& slot) {
        if (slot.turns <= 0 || --slot.turns != 0)
            return;
        auto strike = resolveDelayedStrike(*data, slot);
        residualMessages.insert(residualMessages.end(), strike.messages.begin(), strike.messages.end());
        if (!strike.messages.empty())
        {
            bool doom = slot.moveId.find("DOOM") != std::string::npos || slot.moveId.find("doom") != std::string::npos;
            battleUi()->startBattleAnimationScript(slot.userIsPlayer, slot.hitsPlayer,
                                           doom ? "General_DoomDesireHit" : "General_FutureSightHit");
        }
        if (strike.fainted)
        {
            if (strike.hitsPlayer)
                playerFaintedFromResidual = true;
            else
                opponentFaintedFromResidual = true;
        }
    };
    fireDelayed(data->playerDelayed);
    fireDelayed(data->opponentDelayed);

    // Wish after weather chip / Future Sight (Gen 7+ residual order).
    applyWish(data->playerPkmn, data->playerBattler, data->playerWishTurns, data->playerWishHeal);
    applyWish(data->opponentPkmn, data->opponentBattler, data->opponentWishTurns, data->opponentWishHeal);

    // Clear single-turn protect / flinch / endure
    data->playerBattler.protect      = false;
    data->opponentBattler.protect    = false;
    data->playerBattler.protectKind  = BattleBattler::ProtectKind::None;
    data->opponentBattler.protectKind = BattleBattler::ProtectKind::None;
    data->playerWideGuard = data->opponentWideGuard = false;
    data->playerQuickGuard = data->opponentQuickGuard = false;
    data->playerCraftyShield = data->opponentCraftyShield = false;
    data->playerMatBlock = data->opponentMatBlock = false;
    data->playerSnatch = data->opponentSnatch = false;
    data->playerVoluntarySwitch = data->opponentVoluntarySwitch = false;
    if (!data->playerEchoedThisTurn)
        data->playerEchoedVoice = 0;
    if (!data->opponentEchoedThisTurn)
        data->opponentEchoedVoice = 0;
    data->playerEchoedThisTurn = data->opponentEchoedThisTurn = false;
    data->ionDeluge = false;
    data->playerBattler.movesBecomeElectric = false;
    data->opponentBattler.movesBecomeElectric = false;
    bool playerUpBeforeAbilities    = data->playerPkmn && !data->playerPkmn->isKO();
    bool opponentUpBeforeAbilities = data->opponentPkmn && !data->opponentPkmn->isKO();
    {
        size_t ps = battleSpeed(data->playerPkmn, &data->playerBattler, data->fieldWeather(), false);
        size_t os = battleSpeed(data->opponentPkmn, &data->opponentBattler, data->fieldWeather(), false);
        bool playerFirstAbility = ps >= os;
        for (bool playerSide : {playerFirstAbility, !playerFirstAbility})
        {
            AbilityEvent event;
            event.playerSide = playerSide;
            triggerAbility(*data, AbilityMoment::Residual, event);
            residualMessages.insert(residualMessages.end(), event.messages.begin(), event.messages.end());
        }
    }
    if (playerUpBeforeAbilities && data->playerPkmn->isKO())
        playerFaintedFromResidual = true;
    if (opponentUpBeforeAbilities && data->opponentPkmn->isKO())
        opponentFaintedFromResidual = true;
    auto volatileTick = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler) {
        if (!pkmn || pkmn->isKO())
            return;
        bool magicGuard = !battler.abilitySuppressed && battler.abilityId == "MAGICGUARD";
        if (battler.airborneTurns > 0 && --battler.airborneTurns == 0)
            battler.telekinesis = false;
        if (battler.soundBlockTurns > 0)
            --battler.soundBlockTurns;
        if (battler.disableTurns > 0 && --battler.disableTurns == 0)
        {
            battler.disabledMoveId.clear();
            residualMessages.push_back(
                (boost::format(lc::translate("%1% is no longer disabled!")) % pkmn->getDisplayName()).str());
        }
        if (battler.tauntTurns > 0)
            --battler.tauntTurns;
        ++battler.turnsOnField;
        battler.switchedInThisTurn = false;
        if (battler.multiTurnLock > 0 && --battler.multiTurnLock == 0)
        {
            battler.rampageMoveId.clear();
            battler.rolloutMoveId.clear();
            battler.rolloutPower = 0;
            if (battler.rampageConfuseAtEnd)
            {
                battler.rampageConfuseAtEnd = false;
                bool ownTempo = !battler.abilitySuppressed && battler.abilityId == "OWNTEMPO";
                ConfusionStatus& confusion = pkmn == data->playerPkmn ? data->playerConfusion : data->opponentConfusion;
                if (ownTempo || !confusion.apply())
                    residualMessages.push_back(
                        (boost::format(lc::translate("%1%'s rampage ended.")) % pkmn->getDisplayName()).str());
                else
                {
                    residualMessages.push_back(
                        (boost::format(lc::translate("%1% became confused due to fatigue!")) % pkmn->getDisplayName())
                            .str());
                    bool fatiguePlayer = pkmn == data->playerPkmn;
                    battleUi()->startBattleAnimationScript(fatiguePlayer, fatiguePlayer, "Status_Confusion");
                }
            }
        }
        if (battler.uproarTurns > 0 && --battler.uproarTurns == 0)
        {
            battler.uproarMoveId.clear();
            residualMessages.push_back(
                (boost::format(lc::translate("%1% calmed down.")) % pkmn->getDisplayName()).str());
        }
        if (battler.encoreTurns > 0)
        {
            bool encoreAlive = false;
            for (auto const& slot : pkmn->getMoves())
            {
                if (slot && slot->getDefinition() && slot->getDefinition()->getId() == battler.encoreMoveId
                    && slot->getCurrentPP() > 0)
                {
                    encoreAlive = true;
                    break;
                }
            }
            if (!encoreAlive || --battler.encoreTurns == 0)
            {
                battler.encoreTurns = 0;
                battler.encoreMoveId.clear();
                residualMessages.push_back(
                    (boost::format(lc::translate("%1%'s encore ended!")) % pkmn->getDisplayName()).str());
            }
        }
        if (battler.healBlockTurns > 0)
            --battler.healBlockTurns;
        if (battler.embargoTurns > 0)
            --battler.embargoTurns;
        if (battler.nightmare && pkmn->getStatusCondition() != Pkmn::StatusCondition::SLEEP)
            battler.nightmare = false;
        if (battler.nightmare && pkmn->getStatusCondition() == Pkmn::StatusCondition::SLEEP && !magicGuard)
        {
            pkmn->decreaseHP(std::max<size_t>(1, pkmn->getTotalHP() / 4));
            residualMessages.push_back(
                (boost::format(lc::translate("%1% is locked in a nightmare!")) % pkmn->getDisplayName()).str());
            bool nightmarePlayer = &battler == &data->playerBattler;
            battleUi()->startBattleAnimationScript(nightmarePlayer, nightmarePlayer, "Status_Nightmare");
        }
        if (battler.octolock)
        {
            auto markEject = [&](StatChangeResult const& dropped) {
                if (!dropped.ejected)
                    return;
                if (&battler == &data->playerBattler)
                    data->playerEjectSwitch = true;
                else
                    data->opponentEjectSwitch = true;
            };
            markEject(applyStatChange(*data, battler, pkmn, BattleBattler::StageStat::Defense, -1, residualMessages,
                                      true));
            markEject(applyStatChange(*data, battler, pkmn, BattleBattler::StageStat::SpecialDefense, -1,
                                      residualMessages, true));
        }
        battler.magicCoat = false;
        battler.beakBlast = false;
        battler.shellTrap = false;
        battler.powder = false;
        // Gen 7+: Destiny Bond persists until the user moves again; lock only blocks a second Destiny Bond.
        if (battler.destinyBond)
            battler.destinyBondLock = true;
    };
    volatileTick(data->playerPkmn, data->playerBattler);
    volatileTick(data->opponentPkmn, data->opponentBattler);
    data->fusionMoveId.clear();
    data->playerBattler.endure   = false;
    data->opponentBattler.endure = false;
    data->playerBattler.flinched = false;
    data->opponentBattler.flinched = false;

    data->playerBattler.itemsBlocked   = false;
    data->opponentBattler.itemsBlocked = false;

    bool playerFirst = true;
    if (data->playerPkmn && data->opponentPkmn)
    {
        size_t ps = battleSpeed(data->playerPkmn, &data->playerBattler, data->fieldWeather(), false);
        size_t os = battleSpeed(data->opponentPkmn, &data->opponentBattler, data->fieldWeather(), false);
        playerFirst = ps >= os;
    }

    auto applyHeld = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, ConfusionStatus& confusion,
                            bool isPlayer) {
        if (!pkmn || pkmn->isKO())
            return;
        BattleBattler const& other = isPlayer ? data->opponentBattler : data->playerBattler;
        bool unnerve = !other.abilitySuppressed
                    && (other.abilityId == "UNNERVE" || other.abilityId == "ASONECHILLINGNEIGH"
                        || other.abilityId == "ASONEGRIMNEIGH");
        if (unnerve && pkmn->getHeldItem() && pkmn->getHeldItem()->getDefinition()
            && pkmn->getHeldItem()->getDefinition()->getId().find("BERRY") != std::string::npos)
            return;
        auto tick = ItemUtils::tickHeldItem(pkmn, &battler, &confusion, *data);
        if (tick.ejected)
        {
            if (isPlayer)
                data->playerEjectSwitch = true;
            else
                data->opponentEjectSwitch = true;
        }
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

    auto applyFieldHeal = [this](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler) {
        if (!pkmn || pkmn->isKO() || battler.healBlockTurns > 0)
            return;
        auto residualHeal = [&](size_t amount) {
            if (ItemUtils::activeHeldId(pkmn, &battler) == "BIGROOT")
                amount = std::max<size_t>(1, amount * 13 / 10);
            return amount;
        };
        if (battler.aquaRing && pkmn->getHP() < pkmn->getTotalHP())
        {
            size_t amount = residualHeal(std::max<size_t>(1, pkmn->getTotalHP() / 16));
            pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + amount));
            residualMessages.push_back(
                (boost::format(lc::translate("%1% restored HP with its Aqua Ring!")) % pkmn->getDisplayName()).str());
        }
        if (battler.ingrain && pkmn->getHP() < pkmn->getTotalHP())
        {
            size_t amount = residualHeal(std::max<size_t>(1, pkmn->getTotalHP() / 16));
            pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + amount));
            residualMessages.push_back(
                (boost::format(lc::translate("%1% absorbed nutrients!")) % pkmn->getDisplayName()).str());
            bool ingrainPlayer = &battler == &data->playerBattler;
            battleUi()->startBattleAnimationScript(ingrainPlayer, ingrainPlayer, "General_IngrainHeal");
        }
    };

    // Leftovers and field heals, then status, then Leech / bind / curse, then Yawn, then Perish.
    auto runSide = [&](bool isPlayer) {
        if (isPlayer)
        {
            applyFieldHeal(data->playerPkmn, data->playerBattler);
            if (!playerFaintedFromResidual)
                applyHeld(data->playerPkmn, data->playerBattler, data->playerConfusion, true);
            if (!playerFaintedFromResidual)
                applyStatusResidual(data->playerPkmn, data->playerBattler.toxicCounter, true);
            if (!playerFaintedFromResidual)
                applyLeechSeed(data->playerPkmn, data->playerBattler, data->opponentPkmn, true);
            if (!playerFaintedFromResidual)
                applyBinding(data->playerPkmn, data->playerBattler, true);
            if (!playerFaintedFromResidual)
                applyCurse(data->playerPkmn, data->playerBattler, true);
            if (!playerFaintedFromResidual)
                applyYawn(data->playerPkmn, data->playerBattler);
            if (!playerFaintedFromResidual)
                applyPerish(data->playerPkmn, data->playerBattler, true);
        }
        else
        {
            applyFieldHeal(data->opponentPkmn, data->opponentBattler);
            if (!opponentFaintedFromResidual)
                applyHeld(data->opponentPkmn, data->opponentBattler, data->opponentConfusion, false);
            if (!opponentFaintedFromResidual)
                applyStatusResidual(data->opponentPkmn, data->opponentBattler.toxicCounter, false);
            if (!opponentFaintedFromResidual)
                applyLeechSeed(data->opponentPkmn, data->opponentBattler, data->playerPkmn, false);
            if (!opponentFaintedFromResidual)
                applyBinding(data->opponentPkmn, data->opponentBattler, false);
            if (!opponentFaintedFromResidual)
                applyCurse(data->opponentPkmn, data->opponentBattler, false);
            if (!opponentFaintedFromResidual)
                applyYawn(data->opponentPkmn, data->opponentBattler);
            if (!opponentFaintedFromResidual)
                applyPerish(data->opponentPkmn, data->opponentBattler, false);
        }
    };

    if (playerFirst)
    {
        runSide(true);
        runSide(false);
    }
    else
    {
        runSide(false);
        runSide(true);
    }

    if (weatherEnding)
    {
        data->weather         = Map::Weather::NONE;
        data->weatherFromMove = false;
        residualMessages.push_back(lc::translate("The weather returned to normal!"));
    }

    if (playerFaintedFromResidual)
        data->playerFaintedThisTurn = true;
    if (opponentFaintedFromResidual)
        data->opponentFaintedThisTurn = true;

    auto soulHeart = [this](bool foeFainted, Pkmn::PkmnPtr const& survivor, BattleBattler& battler) {
        if (!foeFainted || !survivor || survivor->isKO() || battler.abilitySuppressed || battler.abilityId != "SOULHEART")
            return;
        auto raised = applyStatChange(*data, battler, survivor, BattleBattler::StageStat::SpecialAttack, 1, residualMessages, false);
        if (raised.ejected)
        {
            if (&battler == &data->playerBattler)
                data->playerEjectSwitch = true;
            else
                data->opponentEjectSwitch = true;
        }
    };
    soulHeart(playerFaintedFromResidual, data->opponentPkmn, data->opponentBattler);
    soulHeart(opponentFaintedFromResidual, data->playerPkmn, data->playerBattler);
}

void SingleBattleWeatherPhase::finishTurn()
{
    ++data->battleTurn;

    if (opponentFaintedFromResidual && data->opponentPkmn && data->opponentPkmn->isKO())
    {
        data->opponentConfusion.clear();
        if (playerFaintedFromResidual)
            data->pendingPlayerReplace = true;
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
        if (hasNextPkmn)
            data->replaceAfterResidual = true;
        data->state = hasNextPkmn ? PKMNS : END;
        return;
    }

    if (data->pendingPlayerReplace && data->playerPkmn && data->playerPkmn->isKO())
    {
        data->pendingPlayerReplace = false;
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
        if (hasNextPkmn)
            data->replaceAfterResidual = true;
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

    if (data->weather != Map::Weather::NONE && scene.fieldWeatherAnim())
    {
        step = Step::Weather;
        if (weatherSpeech)
        {
            weatherSpeech->setTexts({scene.fieldWeatherAnim()->getContinuingText()});
            weatherSpeech->reset();
            weatherSpeech->start();
        }
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
        residualSpeech.reset();
        finishTurn();
        return;
    }

    if (step == Step::Weather)
    {
        if (data->weather == Map::Weather::NONE || !scene.fieldWeatherAnim())
        {
            step = Step::Residuals;
        }
        else
        {
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
            tickFormes(*data);
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
    if (residualSpeech)
    {
        residualSpeech->draw(fps, rs);
        return;
    }

    if (weatherSpeech && step == Step::Weather && !weatherSpeech->shouldClose())
        weatherSpeech->draw(fps, rs);
}
