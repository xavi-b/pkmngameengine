#include "moveresolver.h"

#include "abilitybattle.h"
#include "confusionstatus.h"
#include "itemutils.h"
#include "move.h"
#include "moveeffects.h"
#include "singlebattlescene.h"
#include "trainerscenedata.h"
#include "game.h"
#include "statuscondition.h"
#include "utils.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

namespace
{
bool accuracyCheck(MoveEffectContext const& ctx)
{
    auto def = ctx.def;
    size_t accuracy = def->getAccuracy();
    auto code = def->getFunctionCode();
    bool ohko = code == MoveDef::FunctionCode::OHKO || code == MoveDef::FunctionCode::OHKOIce
             || code == MoveDef::FunctionCode::OHKOHitsUndergroundTarget;
    if (ohko && ctx.user && ctx.target)
    {
        if (code == MoveDef::FunctionCode::OHKOIce && hasType(ctx.target, "ICE", &ctx.targetBattler))
            return false;
        if (code == MoveDef::FunctionCode::OHKOIce && !hasType(ctx.user, "ICE", &ctx.userBattler))
            accuracy = 20;
        int adjusted = static_cast<int>(accuracy) + static_cast<int>(ctx.user->getLevel())
                     - static_cast<int>(ctx.target->getLevel());
        accuracy = static_cast<size_t>(std::clamp(adjusted, 0, 100));
    }
    bool noGuard = (!ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "NOGUARD")
                || (!ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "NOGUARD");
    bool rainSure = ctx.data.fieldWeather() == Map::Weather::RAIN
                 && (code == MoveDef::FunctionCode::ParalyzeTargetAlwaysHitsInRainHitsTargetInSky
                     || code == MoveDef::FunctionCode::ConfuseTargetAlwaysHitsInRainHitsTargetInSky);
    if (accuracy == 0 || ctx.userBattler.lockOn || ctx.targetBattler.telekinesis || noGuard || rainSure)
        return true; // never miss

    // Gen III: Acc_stage / Eva_stage
    float acc = ctx.userBattler.accuracyStageMultiplier();
    bool ignoreEvaStages = ctx.targetBattler.ignoreEvasion
                        || (def
                            && def->getFunctionCode() == MoveDef::FunctionCode::IgnoreTargetDefSpDefEvaStatStages);
    float eva = ignoreEvaStages ? 1.f : ctx.targetBattler.evasionStageMultiplier();
    float chance = accuracy * acc / eva;
    if (!ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "COMPOUNDEYES")
        chance *= 1.3f;
    if (!ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "HUSTLE"
        && def->getCategory() == MoveDef::PHYSICAL)
        chance *= 0.8f;
    bool ignoreTarget = moveIgnoresTargetAbility(ctx.userBattler, def->getFunctionCode());
    if (!ignoreTarget && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "WONDERSKIN"
        && def->getCategory() == MoveDef::STATUS)
        chance = std::min(chance, 50.f);
    if (!ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "VICTORYSTAR")
        chance *= 1.1f;
    bool targetConfused = ctx.target == ctx.data.playerPkmn ? ctx.data.playerConfusion.active : ctx.data.opponentConfusion.active;
    if (!ignoreTarget && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "TANGLEDFEET" && targetConfused)
        chance *= 0.5f;
    if (!ignoreTarget && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "SANDVEIL"
        && ctx.data.fieldWeather() == Map::Weather::SANDSTORM)
        chance *= 0.8f;
    if (!ignoreTarget && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "SNOWCLOAK"
        && ctx.data.fieldWeather() == Map::Weather::HAIL)
        chance *= 0.8f;
    chance *= ItemUtils::heldAccuracyMultiplier(ctx.user, ctx.targetBattler.movedThisTurn, &ctx.userBattler);
    if (ctx.userBattler.micleBerry)
        chance *= 1.2f;
    auto heldOf = [](Pkmn::PkmnPtr const& mon, BattleBattler const& battler) {
        return ItemUtils::activeHeldId(mon, &battler);
    };
    if (ctx.target && ctx.target != ctx.user
        && (heldOf(ctx.target, ctx.targetBattler) == "BRIGHTPOWDER" || heldOf(ctx.target, ctx.targetBattler) == "LAXINCENSE"))
        chance *= 0.9f;
    if (ctx.targetBattler.minimized && def->hasFlag(MoveDef::Flag::TramplesMinimize))
        chance = 100;
    if (chance >= 100)
        return true;
    return Utils::randuint(1, 100) <= static_cast<size_t>(chance);
}

void honorSuctionCups(MoveEffectContext& ctx)
{
    auto blocks = [](BattleBattler const& battler) {
        return !battler.abilitySuppressed && battler.abilityId == "SUCTIONCUPS";
    };
    auto keepIn = [&](bool& shouldSwitch, bool& forced, Pkmn::PkmnPtr const& mon, char const* text) {
        shouldSwitch = false;
        forced       = false;
        if (mon)
            ctx.messages.push_back((boost::format(lc::translate(text)) % mon->getDisplayName()).str());
    };
    bool pierce = ctx.def && moveIgnoresTargetAbility(ctx.userBattler, ctx.def->getFunctionCode());
    if (ctx.forcedSwitchTarget && ctx.targetBattler.ingrain)
        keepIn(ctx.shouldSwitchTarget, ctx.forcedSwitchTarget, ctx.target, "%1% is anchored by its roots!");
    else if (ctx.forcedSwitchTarget && blocks(ctx.targetBattler) && !pierce)
        keepIn(ctx.shouldSwitchTarget, ctx.forcedSwitchTarget, ctx.target, "%1%'s Suction Cups keeps it in!");
    if (ctx.forcedSwitchUser && ctx.userBattler.ingrain)
        keepIn(ctx.shouldSwitchUser, ctx.forcedSwitchUser, ctx.user, "%1% is anchored by its roots!");
    else if (ctx.forcedSwitchUser && blocks(ctx.userBattler))
        keepIn(ctx.shouldSwitchUser, ctx.forcedSwitchUser, ctx.user, "%1%'s Suction Cups keeps it in!");
}

bool targetsUser(MoveDef::Target t)
{
    return t == MoveDef::Target::User || t == MoveDef::Target::UserSide || t == MoveDef::Target::UserAndAllies
        || t == MoveDef::Target::UserOrNearAlly || t == MoveDef::Target::AllAllies;
}

void applyImmediateHeldItems(SingleBattleSceneData& data, std::vector<std::string>& messages)
{
    auto restore = [&](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler) {
        auto tick = ItemUtils::consumeWhiteHerb(pkmn, &battler);
        if (!tick.message.empty())
            messages.push_back(tick.message);
    };
    auto berryTick = [&](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, ConfusionStatus& confusion,
                         bool holderIsPlayer) {
        if (!pkmn || pkmn->isKO())
            return;
        BattleBattler const& other = holderIsPlayer ? data.opponentBattler : data.playerBattler;
        bool unnerve = !other.abilitySuppressed
                    && (other.abilityId == "UNNERVE" || other.abilityId == "ASONECHILLINGNEIGH"
                        || other.abilityId == "ASONEGRIMNEIGH");
        if (unnerve)
            return;
        auto status = ItemUtils::consumeStatusBerry(pkmn, &battler, &confusion);
        if (!status.message.empty())
            messages.push_back(status.message);
        if (status.used)
            return;
        auto tick = ItemUtils::consumePinchBerry(pkmn, &battler, &confusion, data);
        if (!tick.message.empty())
            messages.push_back(tick.message);
        if (tick.ejected)
        {
            if (&battler == &data.playerBattler)
                data.playerEjectSwitch = true;
            else
                data.opponentEjectSwitch = true;
        }
    };
    restore(data.playerPkmn, data.playerBattler);
    restore(data.opponentPkmn, data.opponentBattler);
    berryTick(data.playerPkmn, data.playerBattler, data.playerConfusion, true);
    berryTick(data.opponentPkmn, data.opponentBattler, data.opponentConfusion, false);
}
} // namespace

MoveResolveResult MoveResolver::resolve(SingleBattleSceneData& data, bool playerIsUser, Move::MovePtr const& move,
                                      int callDepth, bool instantTwoTurn)
{
    MoveResolveResult result;
    if (!move || !move->getDefinition())
        return result;

    auto def = move->getDefinition();

    Pkmn::PkmnPtr user   = playerIsUser ? data.playerPkmn : data.opponentPkmn;
    Pkmn::PkmnPtr target = playerIsUser ? data.opponentPkmn : data.playerPkmn;
    BattleBattler& userBattler   = playerIsUser ? data.playerBattler : data.opponentBattler;
    BattleBattler& targetBattler = playerIsUser ? data.opponentBattler : data.playerBattler;

    if (targetsUser(def->getTarget()))
        target = user;

    if (!user)
        return result;

    bool meFirstThisMove = userBattler.meFirstBoost;
    userBattler.meFirstBoost = false;

    data.playerBattler.itemsBlocked   = false;
    data.opponentBattler.itemsBlocked = false;

    auto choiceItem = [&]() {
        if (userBattler.itemsBlocked)
            return std::string();
        if (!user->getHeldItem() || !user->getHeldItem()->getDefinition())
            return std::string();
        return user->getHeldItem()->getDefinition()->getId();
    };
    std::string heldChoice = choiceItem();
    bool gorilla = !userBattler.abilitySuppressed && userBattler.abilityId == "GORILLATACTICS";
    bool klutz   = !userBattler.abilitySuppressed && userBattler.abilityId == "KLUTZ";
    bool        choiceLocked = gorilla || (!klutz && (heldChoice == "CHOICEBAND" || heldChoice == "CHOICESPECS" || heldChoice == "CHOICESCARF"));
    if (userBattler.multiTurnLock > 0 && def && !userBattler.rolloutMoveId.empty() && userBattler.rolloutMoveId != def->getId())
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% is locked into the rollout!")) % user->getDisplayName()).str());
        return result;
    }
    if (userBattler.multiTurnLock > 0 && def && !userBattler.rampageMoveId.empty() && userBattler.rampageMoveId != def->getId())
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% is locked into a rampage!")) % user->getDisplayName()).str());
        return result;
    }
    if (userBattler.bideActive && def
        && def->getFunctionCode() != MoveDef::FunctionCode::MultiTurnAttackBideThenReturnDoubleDamage)
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% is locked into Bide!")) % user->getDisplayName()).str());
        return result;
    }
    if (userBattler.uproarTurns > 0 && def && !userBattler.uproarMoveId.empty() && userBattler.uproarMoveId != def->getId())
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% is locked into the uproar!")) % user->getDisplayName()).str());
        return result;
    }
    if (userBattler.encoreTurns > 0 && def && !userBattler.encoreMoveId.empty() && userBattler.encoreMoveId != def->getId())
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% must use %2%!")) % user->getDisplayName() % userBattler.encoreMoveId).str());
        return result;
    }
    if (userBattler.disableTurns > 0 && def && userBattler.disabledMoveId == def->getId())
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1%'s %2% is disabled!")) % user->getDisplayName() % def->getName()).str());
        return result;
    }
    if (choiceLocked && !userBattler.choiceLock.empty() && def && userBattler.choiceLock != def->getId())
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% can only use %2%!")) % user->getDisplayName() % userBattler.choiceLock)
                .str());
        return result;
    }

    bool moveExecuted = false;
    MoveEffectContext ctx(data, playerIsUser, user, target, userBattler, targetBattler, move, def);
    ctx.instantTwoTurn = instantTwoTurn;
    ctx.callDepth      = callDepth;
    ctx.callDepth = callDepth;

    auto noteMove = [&](bool failed) {
        userBattler.movedThisTurn  = true;
        userBattler.lastMoveFailed = failed;
        if (def)
        {
            // Shell Trap / Beak Blast arm ends when the user attempts the move (success or fail).
            if (def->getFunctionCode() == MoveDef::FunctionCode::UsedAfterUserTakesPhysicalDamage)
                userBattler.shellTrap = false;
            if (def->getFunctionCode() == MoveDef::FunctionCode::BurnAttackerBeforeUserActs)
                userBattler.beakBlast = false;
            if (moveExecuted)
            {
                userBattler.lastMoveId = def->getId();
                ++userBattler.activeMoveActions;
            }
            bool echoed = def->getFunctionCode() == MoveDef::FunctionCode::PowerHigherWithConsecutiveUseOnUserSide;
            if (echoed && !failed)
            {
                int& echo = playerIsUser ? data.playerEchoedVoice : data.opponentEchoedVoice;
                bool& used = playerIsUser ? data.playerEchoedThisTurn : data.opponentEchoedThisTurn;
                int base = echo > 0 ? echo : static_cast<int>(def->getPower());
                echo = std::min(200, base + 40);
                used = true;
            }
            bool chain = def->getFunctionCode() == MoveDef::FunctionCode::PowerHigherWithConsecutiveUse;
            if (failed || !chain)
            {
                userBattler.furyCutterPower = 0;
                userBattler.furyCutterMoveId.clear();
            }
            else if (userBattler.furyCutterMoveId == def->getId() && userBattler.furyCutterPower > 0)
                userBattler.furyCutterPower = std::min(160, userBattler.furyCutterPower * 2);
            else
            {
                userBattler.furyCutterMoveId = def->getId();
                userBattler.furyCutterPower  = std::min(160, static_cast<int>(def->getPower()) * 2);
            }
            if (failed)
            {
                userBattler.metronomeMoveId.clear();
                userBattler.metronomeCount = 0;
            }
            else if (userBattler.metronomeMoveId == def->getId())
                userBattler.metronomeCount = std::min(4, userBattler.metronomeCount + 1);
            else
            {
                userBattler.metronomeMoveId = def->getId();
                userBattler.metronomeCount  = 0;
            }
            if (!failed && choiceLocked)
                userBattler.choiceLock = def->getId();
            if (!failed && (def->getId() == "FUSIONFLARE" || def->getId() == "FUSIONBOLT"))
                data.fusionMoveId = def->getId();
            if (!failed && def->getFunctionCode() == MoveDef::FunctionCode::StartRaiseUserAtk1WhenDamaged)
                userBattler.rage = true;
            else
                userBattler.rage = false;
            if (def->getFunctionCode() != MoveDef::FunctionCode::AttackerFaintsIfUserFaints)
            {
                userBattler.destinyBondLock = false;
                userBattler.destinyBond     = false;
            }
            bool protecting = false;
            switch (def->getFunctionCode())
            {
            case MoveDef::FunctionCode::ProtectUser:
            case MoveDef::FunctionCode::ProtectUserBanefulBunker:
            case MoveDef::FunctionCode::ProtectUserFromDamagingMovesKingsShield:
            case MoveDef::FunctionCode::ProtectUserFromDamagingMovesObstruct:
            case MoveDef::FunctionCode::ProtectUserFromTargetingMovesSpikyShield:
            case MoveDef::FunctionCode::ProtectUserSideFromMultiTargetDamagingMoves:
            case MoveDef::FunctionCode::ProtectUserSideFromPriorityMoves:
            case MoveDef::FunctionCode::ProtectUserSideFromStatusMoves:
            case MoveDef::FunctionCode::ProtectUserSideFromDamagingMovesIfUserFirstTurn:
            case MoveDef::FunctionCode::UserEnduresFaintingThisTurn:
                protecting = true;
                break;
            default:
                break;
            }
            if (!protecting)
                userBattler.protectStreak = 0;
            if (!failed)
                data.lastMoveUsedId = def->getId();
            if (moveExecuted)
            {
                auto& used = userBattler.usedMoveIds;
                if (std::find(used.begin(), used.end(), def->getId()) == used.end())
                    used.push_back(def->getId());
                // Throat Spray only on a successful sound move (not on fail / miss).
                if (!failed && def->hasFlag(MoveDef::Flag::Sound)
                    && ItemUtils::activeHeldId(user, &userBattler) == "THROATSPRAY")
                {
                    auto raised = applyStatChange(data, userBattler, user, BattleBattler::StageStat::SpecialAttack, 1,
                                                  result.messages, false);
                    if (raised.changed)
                    {
                        ItemUtils::noteHeldItemLoss(user, &userBattler, "THROATSPRAY");
                        user->setHeldItem(nullptr);
                    }
                    if (raised.ejected)
                        result.shouldSwitchUser = true;
                }
                size_t before = ctx.messages.size();
                noteMoveCommit(ctx);
                if (ctx.messages.size() > before)
                    result.messages.insert(result.messages.end(), ctx.messages.begin() + before, ctx.messages.end());
                if (ctx.shouldSwitchUser)
                    result.shouldSwitchUser = true;
                if (ctx.shouldSwitchTarget)
                    result.shouldSwitchTarget = true;
            }
        }
    };

    {
        AbilityEvent event;
        event.playerSide = playerIsUser;
        triggerAbility(data, AbilityMoment::BeforeMove, event);
        if (event.blocked)
        {
            result.messages.insert(result.messages.end(), event.messages.begin(), event.messages.end());
            noteMove(true);
            return result;
        }
    }

    // Recharge turn
    if (userBattler.recharging)
    {
        userBattler.recharging = false;
        result.messages.push_back(
            (boost::format(lc::translate("%1% must recharge!")) % user->getDisplayName()).str());
        noteMove(true);
        return result;
    }

    if (userBattler.skyDropLocked)
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% is trapped in the sky!")) % user->getDisplayName()).str());
        noteMove(true);
        return result;
    }

    if (!userBattler.abilitySuppressed && (userBattler.abilityId == "PROTEAN" || userBattler.abilityId == "LIBERO"))
    {
        std::string proteanType = def->getType();
        if (data.ionDeluge && proteanType == "NORMAL")
            proteanType = "ELECTRIC";
        if (userBattler.movesBecomeElectric)
            proteanType = "ELECTRIC";
        userBattler.typesOverride = {proteanType};
    }

    // Two-turn charging first turn: apply effect without damaging
    bool finishingTwoTurn = isTwoTurnChargingMove(def->getFunctionCode()) && userBattler.charging;
    if (isTwoTurnChargingMove(def->getFunctionCode()) && !userBattler.charging)
    {
        // Solar Beam etc. skip charge in sun; Power Herb / Sleep Talk finish in one turn.
        bool skipCharge = instantTwoTurn;
        if (!skipCharge && ItemUtils::activeHeldId(user, &userBattler) == "POWERHERB")
        {
            ItemUtils::noteHeldItemLoss(user, &userBattler, "POWERHERB");
            user->setHeldItem(nullptr);
            result.messages.push_back(
                (boost::format(lc::translate("%1% used its Power Herb!")) % user->getDisplayName()).str());
            skipCharge = true;
        }
        if (!skipCharge)
        {
            move->decreasePP();
            moveExecuted = true;
            applyMoveFunctionCode(ctx);
            result.messages = ctx.messages;
            result.statRose         = ctx.statRose;
            result.statRoseOnTarget = ctx.statRoseOnTarget;
            result.healed = ctx.healed;
            noteMove(false);
            return result;
        }
        ctx.instantTwoTurn = true;
    }

    if (!finishingTwoTurn)
    {
        move->decreasePP();
        moveExecuted = true;
        if (target && !targetBattler.abilitySuppressed && targetBattler.abilityId == "PRESSURE")
            move->decreasePP();
    }

    if (userBattler.activeMoveActions > 0
        && (def->getFunctionCode() == MoveDef::FunctionCode::FailsIfNotUserFirstTurn
            || def->getFunctionCode() == MoveDef::FunctionCode::FlinchTargetFailsIfNotUserFirstTurn))
    {
        result.messages.push_back(lc::translate("But it failed!"));
        noteMove(true);
        return result;
    }

    if (finishingTwoTurn)
    {
        userBattler.charging         = false;
        userBattler.semiKind         = BattleBattler::SemiKind::None;
        userBattler.semiInvulnerable = false;
    }
    struct ReleaseOnReturn
    {
        SingleBattleSceneData* data = nullptr;
        bool                   carrierIsPlayer = false;
        bool                   armed = false;
        ~ReleaseOnReturn()
        {
            if (armed && data)
                releaseSkyDrop(*data, carrierIsPlayer);
        }
    } releaseOnReturn;
    if (finishingTwoTurn && def->getFunctionCode() == MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyTargetCannotAct)
    {
        releaseOnReturn.data = &data;
        releaseOnReturn.carrierIsPlayer = playerIsUser;
        releaseOnReturn.armed = true;
    }

    if (def->hasFlag(MoveDef::Flag::ThawsUser)
        && user->getStatusCondition() == Pkmn::StatusCondition::FREEZE)
    {
        user->setStatusCondition(Pkmn::StatusCondition::NONE);
        result.messages.push_back(
            (boost::format(lc::translate("%1% thawed out!")) % user->getDisplayName()).str());
    }

    // Self-targeting status moves (Rest, setup) skip accuracy vs foe
    bool needsAccuracy = target && target != user && def->getTarget() != MoveDef::Target::User;

    auto blockedBySide = [&]() -> bool {
        if (!target || target == user)
            return false;
        bool defenderIsPlayer = target == data.playerPkmn;
        auto targetKind       = def->getTarget();
        bool multiTarget      = targetKind == MoveDef::Target::AllNearFoes || targetKind == MoveDef::Target::AllBattlers
                        || targetKind == MoveDef::Target::AllNearOthers;
        if (multiTarget && (defenderIsPlayer ? data.playerWideGuard : data.opponentWideGuard)
            && def->getCategory() != MoveDef::STATUS)
            return true;
        if (movePriority(def, user, userBattler, data) > 0
            && (defenderIsPlayer ? data.playerQuickGuard : data.opponentQuickGuard))
            return true;
        if (def->getCategory() == MoveDef::STATUS && !targetsUser(def->getTarget())
            && (defenderIsPlayer ? data.playerCraftyShield : data.opponentCraftyShield))
            return true;
        if (def->getCategory() != MoveDef::STATUS
            && (defenderIsPlayer ? data.playerMatBlock : data.opponentMatBlock))
            return true;
        return false;
    };

    bool breaksProtect = def->getFunctionCode() == MoveDef::FunctionCode::RemoveProtections
                      || def->getFunctionCode() == MoveDef::FunctionCode::RemoveProtectionsBypassSubstitute
                      || def->getFunctionCode() == MoveDef::FunctionCode::HoopaRemoveProtectionsBypassSubstituteLowerUserDef1
                      || def->getFunctionCode() == MoveDef::FunctionCode::TwoTurnAttackInvulnerableRemoveProtections;
    auto crashOnFail = [&]() {
        if (!def || def->getFunctionCode() != MoveDef::FunctionCode::CrashDamageIfFailsUnusableInGravity)
            return;
        if (!userBattler.abilitySuppressed && userBattler.abilityId == "MAGICGUARD")
            return;
        user->decreaseHP(1);
        result.messages.push_back(
            (boost::format(lc::translate("%1% kept going and crashed!")) % user->getDisplayName()).str());
        userBattler.tookDamageThisTurn = true;
        result.userFainted             = user->isKO();
    };
    auto halfHpCostOnFail = [&]() {
        if (!def)
            return;
        auto code = def->getFunctionCode();
        if (code != MoveDef::FunctionCode::UserLosesHalfOfTotalHP
            && code != MoveDef::FunctionCode::UserLosesHalfOfTotalHPExplosive)
            return;
        if (!userBattler.abilitySuppressed && userBattler.abilityId == "MAGICGUARD")
            return;
        user->decreaseHP(std::max<size_t>(1, (user->getTotalHP() + 1) / 2));
        userBattler.tookDamageThisTurn = true;
        result.userFainted             = user->isKO();
    };
    auto selfKoOnFail = [&]() {
        if (!def)
            return;
        auto code = def->getFunctionCode();
        if (code != MoveDef::FunctionCode::UserFaintsExplosive
            && code != MoveDef::FunctionCode::UserFaintsPowersUpInMistyTerrainExplosive)
            return;
        user->setHP(0);
        result.userFainted = true;
    };
    if (blockedBySide())
    {
        result.messages.push_back(lc::translate("The move was blocked!"));
        halfHpCostOnFail();
        selfKoOnFail();
        noteMove(true);
        return result;
    }
    bool pads = ItemUtils::activeHeldId(user, &userBattler) == "PROTECTIVEPADS";
    bool contactMove = def->hasFlag(MoveDef::Flag::Contact)
                    && (userBattler.abilitySuppressed || userBattler.abilityId != "LONGREACH") && !pads;
    bool unseenFist = contactMove && !userBattler.abilitySuppressed && userBattler.abilityId == "UNSEENFIST";
    bool blockedByProtect = targetBattler.protect && needsAccuracy && def->hasFlag(MoveDef::Flag::CanProtect)
                         && !breaksProtect && !unseenFist;
    // King's Shield blocks all damaging moves; status moves pass through (Gen 7–8).
    if (blockedByProtect && targetBattler.protectKind == BattleBattler::ProtectKind::KingsShield
        && def->getCategory() == MoveDef::STATUS)
        blockedByProtect = false;
    if (blockedByProtect && targetBattler.protectKind == BattleBattler::ProtectKind::Obstruct
        && def->getCategory() == MoveDef::STATUS)
        blockedByProtect = false;
    if (blockedByProtect)
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% protected itself!")) % target->getDisplayName()).str());
        crashOnFail();
        halfHpCostOnFail();
        selfKoOnFail();
        if (contactMove)
        {
            switch (targetBattler.protectKind)
            {
            case BattleBattler::ProtectKind::KingsShield: {
                auto dropped = applyStatChange(data, userBattler, user, BattleBattler::StageStat::Attack, -1,
                                               result.messages, true);
                if (dropped.ejected)
                    result.shouldSwitchUser = true;
                break;
            }
            case BattleBattler::ProtectKind::BanefulBunker:
                if (applyStatusCondition(user, Pkmn::StatusCondition::POISON, userBattler.toxicCounter))
                    result.messages.push_back(
                        (boost::format(lc::translate("%1% was poisoned!")) % user->getDisplayName()).str());
                break;
            case BattleBattler::ProtectKind::Obstruct: {
                auto dropped = applyStatChange(data, userBattler, user, BattleBattler::StageStat::Defense, -2,
                                               result.messages, true);
                if (dropped.ejected)
                    result.shouldSwitchUser = true;
                break;
            }
            case BattleBattler::ProtectKind::SpikyShield:
                if (userBattler.abilitySuppressed || userBattler.abilityId != "MAGICGUARD")
                {
                    user->decreaseHP(std::max<size_t>(1, user->getTotalHP() / 8));
                    userBattler.tookDamageThisTurn = true;
                    result.messages.push_back(
                        (boost::format(lc::translate("%1% was hurt!")) % user->getDisplayName()).str());
                    result.userFainted = user->isKO();
                }
                break;
            case BattleBattler::ProtectKind::Normal:
            case BattleBattler::ProtectKind::None:
                break;
            }
        }
        noteMove(true);
        return result;
    }

    if (moveBlockedBeforeHit(ctx))
    {
        result.messages.insert(result.messages.end(), ctx.messages.begin(), ctx.messages.end());
        result.statRose         = result.statRose || ctx.statRose;
        result.statRoseOnTarget = result.statRoseOnTarget || ctx.statRoseOnTarget;
        result.healed = result.healed || ctx.healed;
        result.userFainted = user->isKO();
        noteMove(true);
        return result;
    }

    bool magicBounce = !targetBattler.abilitySuppressed && targetBattler.abilityId == "MAGICBOUNCE";
    auto bounceTarget = def->getTarget();
    bool bounceable = bounceTarget != MoveDef::Target::BothSides && bounceTarget != MoveDef::Target::User
                   && bounceTarget != MoveDef::Target::UserSide && bounceTarget != MoveDef::Target::UserAndAllies
                   && bounceTarget != MoveDef::Target::AllBattlers && bounceTarget != MoveDef::Target::AllAllies;
    if (callDepth == 0 && (targetBattler.magicCoat || magicBounce) && bounceable && def->getCategory() == MoveDef::STATUS
        && target && target != user)
    {
        result.messages.push_back(lc::translate("The status move was bounced back!"));
        targetBattler.magicCoat = false;
        noteMove(false);
        auto bouncedMove = std::make_shared<Move>(def);
        bouncedMove->setCurrentPP(1);
        auto bounced = MoveResolver::resolve(data, !playerIsUser, bouncedMove, callDepth + 1);
        result.messages.insert(result.messages.end(), bounced.messages.begin(), bounced.messages.end());
        result.shouldSwitchUser   = bounced.shouldSwitchTarget;
        result.shouldSwitchTarget = bounced.shouldSwitchUser;
        result.forcedSwitchUser   = bounced.forcedSwitchTarget;
        result.forcedSwitchTarget = bounced.forcedSwitchUser;
        result.userFainted        = bounced.targetFainted;
        result.targetFainted      = bounced.userFainted;
        return result;
    }

    if (needsAccuracy && target && !accuracyCheck(ctx))
    {
        userBattler.micleBerry = false;
        result.messages.push_back(lc::translate("The attack missed!"));
        if (ItemUtils::activeHeldId(user, &userBattler) == "BLUNDERPOLICY")
        {
            auto raised = applyStatChange(data, userBattler, user, BattleBattler::StageStat::Speed, 2, result.messages, false);
            if (raised.changed)
            {
                ItemUtils::noteHeldItemLoss(user, &userBattler, "BLUNDERPOLICY");
                user->setHeldItem(nullptr);
            }
            if (raised.ejected)
                result.shouldSwitchUser = true;
        }
        crashOnFail();
        halfHpCostOnFail();
        selfKoOnFail();
        noteMove(true);
        return result;
    }
    if (needsAccuracy)
        userBattler.micleBerry = false;
    if (userBattler.lockOn)
        userBattler.lockOn = false;

    if (def->getFunctionCode() == MoveDef::FunctionCode::AttackTwoTurnsLater)
    {
        bool ok = applyMoveFunctionCode(ctx);
        result.messages.insert(result.messages.end(), ctx.messages.begin(), ctx.messages.end());
        result.statRose         = result.statRose || ctx.statRose;
        result.statRoseOnTarget = result.statRoseOnTarget || ctx.statRoseOnTarget;
        result.healed = result.healed || ctx.healed;
        result.hit = ok;
        noteMove(!ok);
        return result;
    }

    result.hit = true;
    ctx.moveHit = true;

    auto skipsFormula = [](MoveDef::FunctionCode code) {
        switch (code)
        {
        case MoveDef::FunctionCode::FixedDamage20:
        case MoveDef::FunctionCode::FixedDamage40:
        case MoveDef::FunctionCode::FixedDamageHalfTargetHP:
        case MoveDef::FunctionCode::FixedDamageUserLevel:
        case MoveDef::FunctionCode::FixedDamageUserLevelRandom:
        case MoveDef::FunctionCode::LowerTargetHPToUserHP:
        case MoveDef::FunctionCode::OHKO:
        case MoveDef::FunctionCode::OHKOIce:
        case MoveDef::FunctionCode::OHKOHitsUndergroundTarget:
        case MoveDef::FunctionCode::CounterPhysicalDamage:
        case MoveDef::FunctionCode::CounterSpecialDamage:
        case MoveDef::FunctionCode::CounterDamagePlusHalf:
        case MoveDef::FunctionCode::UserFaintsFixedDamageUserHP:
            return true;
        default:
            return false;
        }
    };
    bool directDamage = skipsFormula(def->getFunctionCode());
    if (userBattler.bideActive || def->getFunctionCode() == MoveDef::FunctionCode::MultiTurnAttackBideThenReturnDoubleDamage
        || directDamage)
    {
        if (directDamage && !target)
        {
            noteMove(true);
            return result;
        }
        bool ok = applyMoveFunctionCode(ctx);
        result.messages.insert(result.messages.end(), ctx.messages.begin(), ctx.messages.end());
        result.statRose         = result.statRose || ctx.statRose;
        result.statRoseOnTarget = result.statRoseOnTarget || ctx.statRoseOnTarget;
        result.healed = result.healed || ctx.healed;
        if (directDamage)
            result.hit = ok;
        result.userFainted   = user->isKO();
        result.targetFainted = target && target->isKO();
        if (result.hit && target && target->isKO() && targetBattler.destinyBond && user && !user->isKO())
        {
            targetBattler.destinyBond = false;
            user->setHP(0);
            result.userFainted = true;
            result.messages.push_back(lc::translate("It took its attacker down with it!"));
        }
        applyImmediateHeldItems(data, result.messages);
        noteMove(directDamage && !ok);
        return result;
    }

    // Pure status / self moves without damage
    if (def->getCategory() == MoveDef::STATUS || def->getPower() == 0)
    {
        bool foeSnatch = playerIsUser ? data.opponentSnatch : data.playerSnatch;
        if (foeSnatch && targetsUser(def->getTarget()))
        {
            if (playerIsUser)
                data.opponentSnatch = false;
            else
                data.playerSnatch = false;
            result.messages.push_back(lc::translate("The move was snatched!"));
            Pkmn::PkmnPtr  foe        = playerIsUser ? data.opponentPkmn : data.playerPkmn;
            BattleBattler& foeBattler = playerIsUser ? data.opponentBattler : data.playerBattler;
            MoveEffectContext swapped(data, !playerIsUser, foe, foe, foeBattler, foeBattler, move, def);
            swapped.callDepth = callDepth;
            applyMoveFunctionCode(swapped);
            result.messages.insert(result.messages.end(), swapped.messages.begin(), swapped.messages.end());
            applyImmediateHeldItems(data, result.messages);
            noteMove(false);
            return result;
        }
    }
    if (def->getCategory() == MoveDef::STATUS || def->getPower() == 0)
    {
        auto tgt = def->getTarget();
        bool sideMove = tgt == MoveDef::Target::FoeSide || tgt == MoveDef::Target::BothSides || targetsUser(tgt);
        bool bypassSub = def->getFunctionCode() == MoveDef::FunctionCode::RemoveProtectionsBypassSubstitute
                      || def->getFunctionCode() == MoveDef::FunctionCode::HoopaRemoveProtectionsBypassSubstituteLowerUserDef1;
        if (!sideMove && target && target != user && targetBattler.substituteHP > 0 && !bypassSub)
        {
            result.messages.push_back(lc::translate("But it failed!"));
            noteMove(true);
            return result;
        }
        bool ok = applyMoveFunctionCode(ctx);
        result.messages.insert(result.messages.end(), ctx.messages.begin(), ctx.messages.end());
        result.statRose         = result.statRose || ctx.statRose;
        result.statRoseOnTarget = result.statRoseOnTarget || ctx.statRoseOnTarget;
        result.healed = result.healed || ctx.healed;
        honorSuctionCups(ctx);
        result.shouldSwitchUser   = ctx.shouldSwitchUser;
        result.shouldSwitchTarget = ctx.shouldSwitchTarget;
        result.forcedSwitchTarget = ctx.forcedSwitchTarget;
        result.forcedSwitchUser   = ctx.forcedSwitchUser;
        result.userFainted        = user->isKO();
        result.targetFainted      = target && target->isKO();
        result.hit                = ok;
        applyImmediateHeldItems(data, result.messages);
        noteMove(!ok);
        return result;
    }

    if (!target)
    {
        noteMove(true);
        return result;
    }

    int hits = moveHitCount(def);
    if (def->getFunctionCode() == MoveDef::FunctionCode::HitTwoToFiveTimesOrThreeForAshGreninja
        && userBattler.battleBondActive)
        hits = 3;
    std::vector<Pkmn::PkmnPtr> beatUp;
    if (def->getFunctionCode() == MoveDef::FunctionCode::HitOncePerUserTeamMember)
    {
        auto consider = [&](auto const& party) {
            for (auto const& member : party)
            {
                if (member && !member->isKO())
                    beatUp.push_back(member);
            }
        };
        if (playerIsUser)
            consider(Game::instance()->data.player.pkmns);
        else if (auto* trainer = dynamic_cast<TrainerSceneData*>(&data))
        {
            if (trainer->opponentTrainer)
                consider(trainer->opponentTrainer->pkmns);
        }
        if (beatUp.empty() && user && !user->isKO())
            beatUp.push_back(user);
        if (!beatUp.empty())
            hits = static_cast<int>(beatUp.size());
    }
    auto fc = def->getFunctionCode();
    bool skillLink = !userBattler.abilitySuppressed && userBattler.abilityId == "SKILLLINK"
                  && (fc == MoveDef::FunctionCode::HitTwoToFiveTimes
                      || fc == MoveDef::FunctionCode::HitTwoToFiveTimesOrThreeForAshGreninja
                      || fc == MoveDef::FunctionCode::HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1);
    if (skillLink)
        hits = 5;
    bool parentalBond = !userBattler.abilitySuppressed && userBattler.abilityId == "PARENTALBOND" && hits == 1
                     && def->getCategory() != MoveDef::STATUS
                     && def->getId() != "STRUGGLE"
                     && fc != MoveDef::FunctionCode::HitOncePerUserTeamMember
                     && fc != MoveDef::FunctionCode::UserFaintsExplosive
                     && fc != MoveDef::FunctionCode::UserFaintsPowersUpInMistyTerrainExplosive
                     && fc != MoveDef::FunctionCode::UserFaintsFixedDamageUserHP
                     && fc != MoveDef::FunctionCode::FixedDamage20
                     && fc != MoveDef::FunctionCode::FixedDamage40
                     && fc != MoveDef::FunctionCode::FixedDamageHalfTargetHP
                     && fc != MoveDef::FunctionCode::FixedDamageUserLevel
                     && fc != MoveDef::FunctionCode::FixedDamageUserLevelRandom
                     && fc != MoveDef::FunctionCode::OHKO
                     && fc != MoveDef::FunctionCode::OHKOIce
                     && fc != MoveDef::FunctionCode::OHKOHitsUndergroundTarget
                     && fc != MoveDef::FunctionCode::CounterPhysicalDamage
                     && fc != MoveDef::FunctionCode::CounterSpecialDamage
                     && fc != MoveDef::FunctionCode::CounterDamagePlusHalf
                     && fc != MoveDef::FunctionCode::ThrowUserItemAtTarget
                     && fc != MoveDef::FunctionCode::MultiTurnAttackPreventSleeping
                     && fc != MoveDef::FunctionCode::MultiTurnAttackPowersUpEachTurn
                     && fc != MoveDef::FunctionCode::MultiTurnAttackConfuseUserAtEnd
                     && fc != MoveDef::FunctionCode::LowerTargetHPToUserHP
                     && fc != MoveDef::FunctionCode::SleepTargetChangeUserMeloettaForm
                     && !isTwoTurnChargingMove(fc);
    if (parentalBond)
        hits = 2;
    size_t totalDamage = 0;
    size_t subDamage   = 0;
    bool   anyCritical = false;
    float  effectiveness = 1.f;
    bool   immune        = false;
    std::string landedType;
    bool   forceCrit     = def->getFunctionCode() == MoveDef::FunctionCode::AlwaysCriticalHit
                       || def->getFunctionCode() == MoveDef::FunctionCode::HitThreeTimesAlwaysCriticalHit;
    if (!userBattler.abilitySuppressed && userBattler.abilityId == "MERCILESS" && target
        && (target->getStatusCondition() == Pkmn::StatusCondition::POISON
            || target->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON))
        forceCrit = true;
    bool pendingLaserFocus = userBattler.laserFocus;
    bool chargedThisMove   = false;
    // Apply -ate / Normalize conversion before gem consume so gem type matches post-conversion.
    bool ateBoost = false;
    if (!userBattler.abilitySuppressed && ctx.typeOverride.empty())
    {
        std::string converted;
        if (def->getType() == "NORMAL" && userBattler.abilityId == "PIXILATE")
            converted = "FAIRY";
        else if (def->getType() == "NORMAL" && userBattler.abilityId == "AERILATE")
            converted = "FLYING";
        else if (def->getType() == "NORMAL" && userBattler.abilityId == "REFRIGERATE")
            converted = "ICE";
        else if (def->getType() == "NORMAL" && userBattler.abilityId == "GALVANIZE")
            converted = "ELECTRIC";
        else if (userBattler.abilityId == "NORMALIZE" && def->getType() != "NORMAL")
            converted = "NORMAL";
        else if (userBattler.abilityId == "NORMALIZE")
            ateBoost = true;
        if (!converted.empty())
        {
            ctx.typeOverride = converted;
            ateBoost = true;
        }
    }
    float gemBoost = 1.f;
    bool gemPending = false;
    std::string pendingGemId;
    std::string gemId = ItemUtils::activeHeldId(user, &userBattler);
    if (!gemId.empty() && def->getCategory() != MoveDef::STATUS)
    {
        char const* gem   = nullptr;
        if (gemId == "FIREGEM") gem = "FIRE";
        else if (gemId == "WATERGEM") gem = "WATER";
        else if (gemId == "ELECTRICGEM") gem = "ELECTRIC";
        else if (gemId == "GRASSGEM") gem = "GRASS";
        else if (gemId == "ICEGEM") gem = "ICE";
        else if (gemId == "FIGHTINGGEM") gem = "FIGHTING";
        else if (gemId == "POISONGEM") gem = "POISON";
        else if (gemId == "GROUNDGEM") gem = "GROUND";
        else if (gemId == "FLYINGGEM") gem = "FLYING";
        else if (gemId == "PSYCHICGEM") gem = "PSYCHIC";
        else if (gemId == "BUGGEM") gem = "BUG";
        else if (gemId == "ROCKGEM") gem = "ROCK";
        else if (gemId == "GHOSTGEM") gem = "GHOST";
        else if (gemId == "DRAGONGEM") gem = "DRAGON";
        else if (gemId == "DARKGEM") gem = "DARK";
        else if (gemId == "STEELGEM") gem = "STEEL";
        else if (gemId == "FAIRYGEM") gem = "FAIRY";
        else if (gemId == "NORMALGEM") gem = "NORMAL";
        std::string gemMoveType = ctx.typeOverride.empty() ? def->getType() : ctx.typeOverride;
        if (gem && gemMoveType == gem)
        {
            gemBoost     = 1.3f;
            gemPending   = true;
            pendingGemId = gemId;
        }
    }
    if (!userBattler.abilitySuppressed && userBattler.abilityId == "STANCECHANGE" && def->getCategory() != MoveDef::STATUS
        && !userBattler.bladeForme && user)
    {
        auto stats = user->getStats();
        userBattler.attackOverride    = static_cast<int>(stats[PkmnDef::DEFENSE]);
        userBattler.spAttackOverride  = static_cast<int>(stats[PkmnDef::SPECIAL_DEFENSE]);
        userBattler.defenseOverride   = static_cast<int>(stats[PkmnDef::ATTACK]);
        userBattler.spDefenseOverride = static_cast<int>(stats[PkmnDef::SPECIAL_ATTACK]);
        userBattler.bladeForme        = true;
    }
    int savedAttackOverride = userBattler.attackOverride;
    for (int hit = 0; hit < hits; ++hit)
    {
        if (hit > 0 && def->getFunctionCode() == MoveDef::FunctionCode::HitThreeTimesPowersUpWithEachHit
            && needsAccuracy && !accuracyCheck(ctx))
        {
            result.messages.push_back(lc::translate("The attack missed!"));
            break;
        }
        userBattler.attackOverride = savedAttackOverride;
        size_t basePower = def->getPower();
        size_t power     = moveEffectPower(ctx, basePower);
        if (meFirstThisMove)
            power = std::max<size_t>(1, power * 3 / 2);
        if (def->getFunctionCode() == MoveDef::FunctionCode::HitThreeTimesPowersUpWithEachHit)
            power *= static_cast<size_t>(hit + 1);
        float screen = 1.f;
        {
            bool defenderIsPlayer = target == data.playerPkmn;
            int  reflect = defenderIsPlayer ? data.playerReflect : data.opponentReflect;
            int  light   = defenderIsPlayer ? data.playerLightScreen : data.opponentLightScreen;
            auto category = def->battleCategory();
            bool breakScreens = def->getFunctionCode() == MoveDef::FunctionCode::RemoveScreens;
            if (!breakScreens && ((category == MoveDef::PHYSICAL && reflect > 0)
                || (category == MoveDef::SPECIAL && light > 0)))
                screen = 0.5f;
        }
        std::string typeOverride = ctx.typeOverride;
        if (!userBattler.abilitySuppressed && userBattler.abilityId == "LIQUIDVOICE" && def->hasFlag(MoveDef::Flag::Sound)
            && typeOverride.empty())
            typeOverride = "WATER";
        if (data.ionDeluge && (typeOverride.empty() ? def->getType() : typeOverride) == "NORMAL")
            typeOverride = "ELECTRIC";
        if (userBattler.movesBecomeElectric)
            typeOverride = "ELECTRIC";
        if (def->getFunctionCode() == MoveDef::FunctionCode::RandomlyDamageOrHealTarget)
        {
            size_t roll = Utils::randuint(1, 100);
            if (roll > 80)
            {
                // Present heal fails against Substitute.
                if (targetBattler.substituteHP > 0 || targetBattler.healBlockTurns > 0)
                {
                    result.messages.push_back(lc::translate("But it failed!"));
                    break;
                }
                size_t heal = std::max<size_t>(1, target->getTotalHP() / 4);
                target->setHP(std::min(target->getTotalHP(), target->getHP() + heal));
                result.messages.push_back(
                    (boost::format(lc::translate("%1% had its HP restored.")) % target->getDisplayName()).str());
                break;
            }
            if (roll <= 40)
                power = 40;
            else if (roll <= 70)
                power = 80;
            else
                power = 120;
        }
        std::string attackTypeNow = typeOverride.empty() ? def->getType() : typeOverride;
        landedType = attackTypeNow;
        power = std::max<size_t>(1, static_cast<size_t>(static_cast<float>(power)
                * ItemUtils::heldTypePowerMultiplier(user, attackTypeNow, &userBattler)));
        power = std::max<size_t>(1, static_cast<size_t>(static_cast<float>(power)
                * abilityTypePowerMultiplier(user, userBattler, attackTypeNow)));
        auto code = def->getFunctionCode();
        bool ignoreAbility = moveIgnoresTargetAbility(userBattler, code);
        DamageAdjustments adjust;
        adjust.useTargetAttack = code == MoveDef::FunctionCode::UseTargetAttackInsteadOfUserAttack;
        adjust.useUserDefense = code == MoveDef::FunctionCode::UseUserDefenseInsteadOfUserAttack;
        adjust.usePhysicalDefense = code == MoveDef::FunctionCode::UseTargetDefenseInsteadOfTargetSpDef;
        adjust.ignoreDefenseStages = code == MoveDef::FunctionCode::IgnoreTargetDefSpDefEvaStatStages;
        adjust.includeFlying = code == MoveDef::FunctionCode::EffectivenessIncludesFlyingType;
        bool defenderIsPlayer = target == data.playerPkmn;
        adjust.blockCrit = (defenderIsPlayer ? data.playerLuckyChant : data.opponentLuckyChant) > 0;
        if (!ignoreAbility && !targetBattler.abilitySuppressed
            && (targetBattler.abilityId == "BATTLEARMOR" || targetBattler.abilityId == "SHELLARMOR"))
            adjust.blockCrit = true;
        if (!userBattler.abilitySuppressed && userBattler.abilityId == "UNAWARE")
            adjust.ignoreDefenseStages = true;
        if (!ignoreAbility && !targetBattler.abilitySuppressed && targetBattler.abilityId == "UNAWARE")
            adjust.ignoreAttackStages = true;
        if (code == MoveDef::FunctionCode::CategoryDependsOnHigherDamageIgnoreTargetAbility
            || code == MoveDef::FunctionCode::CategoryDependsOnHigherDamagePoisonTarget)
        {
            auto physicalAdjust = adjust;
            auto specialAdjust = adjust;
            physicalAdjust.forcePhysical = true;
            specialAdjust.forceSpecial = true;
            auto physRoll = SingleBattleScene::computeDamage(user, target, move, data.fieldWeather(), &userBattler, &targetBattler,
                                                             power, false, forceCrit, screen, false, typeOverride,
                                                             physicalAdjust);
            auto specRoll = SingleBattleScene::computeDamage(user, target, move, data.fieldWeather(), &userBattler, &targetBattler,
                                                             power, false, forceCrit, screen, false, typeOverride,
                                                             specialAdjust);
            if (physRoll.amount >= specRoll.amount)
                adjust.forcePhysical = true;
            else
                adjust.forceSpecial = true;
            if (code == MoveDef::FunctionCode::CategoryDependsOnHigherDamagePoisonTarget)
            {
                if (adjust.forceSpecial)
                    ignoreAbility = true;
                bool pads = ItemUtils::activeHeldId(user, &userBattler) == "PROTECTIVEPADS";
                bool longReach = !userBattler.abilitySuppressed && userBattler.abilityId == "LONGREACH";
                contactMove = adjust.forcePhysical && !pads && !longReach;
            }
        }
        Map::Weather weather = data.fieldWeather();
        if (data.mudSportTurns > 0 && attackTypeNow == "ELECTRIC")
            adjust.multiplier *= 0.5f;
        if (data.waterSportTurns > 0 && attackTypeNow == "FIRE")
            adjust.multiplier *= 0.5f;
        adjust.multiplier *= abilityOutgoingMultiplier(user, userBattler, def, targetBattler.movedThisTurn, weather, 1.f,
                                                      false, attackTypeNow);
        std::string typed = typeOverride.empty() ? def->getType() : typeOverride;
        if (!userBattler.abilitySuppressed)
        {
            if (ateBoost)
                adjust.multiplier *= 1.2f;
            if (userBattler.abilityId == "TRANSISTOR" && typed == "ELECTRIC")
                adjust.multiplier *= 1.5f;
            if (userBattler.abilityId == "STEELWORKER" && typed == "STEEL")
                adjust.multiplier *= 1.5f;
            if (userBattler.abilityId == "STEELYSPIRIT" && typed == "STEEL")
                adjust.multiplier *= 1.5f;
            if (userBattler.abilityId == "DRAGONSMAW" && typed == "DRAGON")
                adjust.multiplier *= 1.5f;
            if (userBattler.abilityId == "SANDFORCE" && weather == Map::Weather::SANDSTORM
                && (typed == "ROCK" || typed == "GROUND" || typed == "STEEL"))
                adjust.multiplier *= 1.3f;
            if (userBattler.abilityId == "RIVALRY" && target && user->getGender() != Pkmn::Gender::GENDERLESS
                && target->getGender() != Pkmn::Gender::GENDERLESS)
                adjust.multiplier *= user->getGender() == target->getGender() ? 1.25f : 0.75f;
            if (userBattler.abilityId == "STAKEOUT" && targetBattler.switchedInThisTurn)
                adjust.multiplier *= 2.f;
        }
        if (ItemUtils::activeHeldId(user, &userBattler) == "METRONOME" && userBattler.metronomeMoveId == def->getId())
            adjust.multiplier *= std::min(2.f, 1.f + 0.2f * static_cast<float>(userBattler.metronomeCount + 1));
        adjust.multiplier *= gemBoost;
        auto hasAura = [&](std::string const& id) {
            return (!userBattler.abilitySuppressed && userBattler.abilityId == id)
                || (!targetBattler.abilitySuppressed && targetBattler.abilityId == id);
        };
        bool auraBreak = hasAura("AURABREAK");
        if (!beatUp.empty() && hit < static_cast<int>(beatUp.size()) && beatUp[static_cast<size_t>(hit)])
        {
            auto member = beatUp[static_cast<size_t>(hit)];
            size_t baseAtk = 1;
            if (member->getDefinition())
                baseAtk = member->getDefinition()->getBaseStats().at(PkmnDef::ATTACK);
            userBattler.attackOverride = static_cast<int>(std::max<size_t>(1, baseAtk));
            power                      = 1;
            typeOverride               = "TYPELESS";
            adjust.forcePhysical       = true;
        }
        if (typed == "GROUND"
            && (grounded(target, targetBattler, data)
                || code == MoveDef::FunctionCode::HitsTargetInSkyGroundsTarget))
            adjust.pierceGroundImmunity = true;
        if (typed == "DARK" && hasAura("DARKAURA"))
            adjust.multiplier *= auraBreak ? 0.75f : (4.f / 3.f);
        if (typed == "FAIRY" && hasAura("FAIRYAURA"))
            adjust.multiplier *= auraBreak ? 0.75f : (4.f / 3.f);
        auto   roll      = SingleBattleScene::computeDamage(user, target, move, weather, &userBattler,
                                                            &targetBattler, power, false, forceCrit, screen,
                                                            false, typeOverride, adjust);
        if (roll.effectiveness > 0.f && code == MoveDef::FunctionCode::UserStealTargetPositiveStatStages
            && !ctx.stagesStolen)
        {
            for (size_t i = 0; i < BattleBattler::StageStatCount; ++i)
            {
                int stage = targetBattler.stages[i];
                if (stage > 0)
                {
                    auto stolen = applyStatChange(data, userBattler, user, static_cast<BattleBattler::StageStat>(i),
                                                  stage, result.messages, false);
                    targetBattler.stages[i] = 0;
                    if (stolen.ejected)
                        ctx.shouldSwitchUser = true;
                }
            }
            ctx.stagesStolen = true;
            roll = SingleBattleScene::computeDamage(user, target, move, weather, &userBattler,
                                                    &targetBattler, power, false, forceCrit, screen,
                                                    false, typeOverride, adjust);
        }
        bool contactHit = contactMove;
        roll.amount = static_cast<size_t>(static_cast<float>(roll.amount)
                      * abilityIncomingMultiplier(target, targetBattler, attackTypeNow, roll.effectiveness, roll.critical,
                                                  def->getCategory(), contactHit, ignoreAbility, data.strongWinds));
        {
            bool targetMoved = targetBattler.movedThisTurn;
            float without = abilityOutgoingMultiplier(user, userBattler, def, targetMoved, weather, 1.f, false, attackTypeNow);
            float with    = abilityOutgoingMultiplier(user, userBattler, def, targetMoved, weather, roll.effectiveness,
                                                      roll.critical, attackTypeNow);
            if (without > 0.f)
                roll.amount = static_cast<size_t>(static_cast<float>(roll.amount) * (with / without));
        }
        if (parentalBond && hit > 0 && roll.amount > 1)
            roll.amount = std::max<size_t>(1, roll.amount / 4);
        size_t damage    = roll.amount;
        anyCritical      = anyCritical || roll.critical;
        effectiveness    = roll.effectiveness;
        if (effectiveness <= 0.f)
        {
            immune = true;
            break;
        }
        std::string attackType = typeOverride.empty() ? def->getType() : typeOverride;
        if (ItemUtils::airBalloonBlocksGround(target, attackType, result.messages, &targetBattler))
        {
            immune = true;
            break;
        }
        if (!ignoreAbility && !targetBattler.abilitySuppressed && targetBattler.abilityId == "WONDERGUARD"
            && effectiveness <= 1.f)
        {
            result.messages.push_back(
                (boost::format(lc::translate("%1%'s Wonder Guard protected it!")) % target->getDisplayName()).str());
            immune = true;
            break;
        }
        bool pierceGround = attackType == "GROUND"
                         && (grounded(target, targetBattler, data)
                             || def->getFunctionCode() == MoveDef::FunctionCode::HitsTargetInSkyGroundsTarget);
        if (abilityGrantsImmunity(data, target, targetBattler, attackType, ignoreAbility, result.messages,
                                  pierceGround))
        {
            immune = true;
            break;
        }
        if (userBattler.chargeBoost && attackTypeNow == "ELECTRIC")
            chargedThisMove = true;
        if (pendingLaserFocus)
        {
            userBattler.laserFocus = false;
            pendingLaserFocus      = false;
        }
        if (effectiveness > 1.f && ItemUtils::activeHeldId(user, &userBattler) == "EXPERTBELT")
            damage = std::max<size_t>(1, damage * 6 / 5);
        bool bypassSubstitute = code == MoveDef::FunctionCode::RemoveProtectionsBypassSubstitute
                             || code == MoveDef::FunctionCode::HoopaRemoveProtectionsBypassSubstituteLowerUserDef1;
        auto consumeGem = [&]() {
            if (!gemPending || !user)
                return;
            gemPending = false;
            userBattler.consumedItemId = pendingGemId;
            if (!userBattler.abilitySuppressed && userBattler.abilityId == "UNBURDEN")
                userBattler.unburdenActive = true;
            user->setHeldItem(nullptr);
            result.messages.push_back(
                (boost::format(lc::translate("%1%'s gem boosted the move!")) % user->getDisplayName()).str());
        };
        if (targetBattler.substituteHP > 0 && !bypassSubstitute)
        {
            ctx.hitSubstitute = true;
            consumeGem();
            subDamage += damage;
            int sub = targetBattler.substituteHP;
            int dmg = static_cast<int>(damage);
            if (dmg >= sub)
            {
                targetBattler.substituteHP = 0;
                result.messages.push_back(lc::translate("The substitute faded!"));
            }
            else
                targetBattler.substituteHP -= dmg;
        }
        else
        {
            damage = ItemUtils::reduceIncomingDamage(target, attackType, effectiveness, damage, result.messages,
                                                     &targetBattler);
            if (code == MoveDef::FunctionCode::CannotMakeTargetFaint && damage >= target->getHP() && target->getHP() > 0)
                damage = target->getHP() - 1;
            size_t hpBefore = target->getHP();
            if (damage > 0)
                consumeGem();
            if (!ignoreAbility && !targetBattler.abilitySuppressed && targetBattler.abilityId == "DISGUISE"
                && !targetBattler.disguiseBroken)
            {
                targetBattler.disguiseBroken = true;
                damage = 0;
                result.messages.push_back(
                    (boost::format(lc::translate("%1%'s Disguise broke!")) % target->getDisplayName()).str());
                if (targetBattler.abilitySuppressed || targetBattler.abilityId != "MAGICGUARD")
                    damage = std::max<size_t>(1, target->getTotalHP() / 8);
            }
            if (!ignoreAbility && !targetBattler.abilitySuppressed && targetBattler.abilityId == "ICEFACE"
                && !targetBattler.iceFaceBroken && def->getCategory() == MoveDef::PHYSICAL)
            {
                targetBattler.iceFaceBroken = true;
                damage = 0;
                result.messages.push_back(
                    (boost::format(lc::translate("%1%'s Ice Face took the hit!")) % target->getDisplayName()).str());
            }
            target->decreaseHP(damage);
            if (damage > 0)
                targetBattler.lastHitType = attackType;
            if (damage > 0)
                spitGulpMissile(ctx);
            if (damage > 0 && !targetBattler.abilitySuppressed)
            {
                auto id = targetBattler.abilityId;
                if (id == "COLORCHANGE")
                    targetBattler.typesOverride = {attackType};
                if (id == "RATTLED" && (attackType == "BUG" || attackType == "GHOST" || attackType == "DARK"))
                {
                    auto raised = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::Speed, 1, result.messages, false);
                    if (raised.ejected)
                        ctx.shouldSwitchTarget = true;
                }
                if (id == "WATERCOMPACTION" && attackType == "WATER")
                {
                    auto raised = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::Defense, 2, result.messages, false);
                    if (raised.ejected)
                        ctx.shouldSwitchTarget = true;
                }
                if (id == "STEAMENGINE" && (attackType == "FIRE" || attackType == "WATER"))
                {
                    auto raised = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::Speed, 6, result.messages, false);
                    if (raised.ejected)
                        ctx.shouldSwitchTarget = true;
                }
                if (id == "SANDSPIT" && !primordialWeatherBlocksNewWeather(data))
                {
                    data.weather         = Map::Weather::SANDSTORM;
                    data.weatherFromMove = true;
                    data.weatherTurns    = ItemUtils::weatherTurnsForHolder(target, Map::Weather::SANDSTORM, &targetBattler);
                    result.messages.push_back(
                        (boost::format(lc::translate("%1%'s Sand Spit whipped up a sandstorm!")) % target->getDisplayName())
                            .str());
                }
                bool crossedHalf = hpBefore * 2 > target->getTotalHP() && target->getHP() * 2 <= target->getTotalHP()
                                && !target->isKO();
                if (crossedHalf && id == "BERSERK")
                {
                    auto raised = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::SpecialAttack, 1,
                                                  result.messages, false);
                    if (raised.ejected)
                        ctx.shouldSwitchTarget = true;
                }
                if (crossedHalf && (id == "WIMPOUT" || id == "EMERGENCYEXIT"))
                    ctx.shouldSwitchTarget = true;
                if (roll.critical && id == "ANGERPOINT")
                {
                    targetBattler.stages[static_cast<size_t>(BattleBattler::StageStat::Attack)] = 6;
                    result.messages.push_back(
                        (boost::format(lc::translate("%1%'s Anger Point maxed its Attack!")) % target->getDisplayName())
                            .str());
                }
            }
            if (damage > 0)
            {
                bool switchHolder = false;
                bool forceOut     = false;
                ItemUtils::applyDamagingHitItems(target, targetBattler, effectiveness, attackType, switchHolder, forceOut,
                                                 result.messages, data);
                std::string flinchItem = ItemUtils::activeHeldId(user, &userBattler);
                if ((flinchItem == "KINGSROCK" || flinchItem == "RAZORFANG")
                    && (ignoreAbility || targetBattler.abilitySuppressed || targetBattler.abilityId != "INNERFOCUS")
                    && Utils::randuint(1, 100) <= 10)
                    targetBattler.flinched = true;
                if (code == MoveDef::FunctionCode::HitTwoTimesFlinchTarget)
                    rollHitFlinch(ctx);
                auto berryId = [&]() { return ItemUtils::activeHeldId(target, &targetBattler); };
                auto eatBerry = [&]() {
                    targetBattler.consumedItemId = berryId();
                    targetBattler.consumedBerry  = true;
                    if (!targetBattler.abilitySuppressed && targetBattler.abilityId == "UNBURDEN")
                        targetBattler.unburdenActive = true;
                    target->setHeldItem(nullptr);
                };
                std::string berry = berryId();
                auto category = def->getCategory();
                bool ripen = !targetBattler.abilitySuppressed && targetBattler.abilityId == "RIPEN";
                bool attackerUnnerve = !userBattler.abilitySuppressed
                                    && (userBattler.abilityId == "UNNERVE" || userBattler.abilityId == "ASONECHILLINGNEIGH"
                                        || userBattler.abilityId == "ASONEGRIMNEIGH");
                if (attackerUnnerve)
                    berry.clear();
                if (berry == "JABOCABERRY" && category == MoveDef::PHYSICAL)
                {
                    if (userBattler.abilitySuppressed || userBattler.abilityId != "MAGICGUARD")
                    {
                        user->decreaseHP(std::max<size_t>(1, user->getTotalHP() / (ripen ? 4 : 8)));
                        result.messages.push_back(
                            (boost::format(lc::translate("%1% was hurt by the Jaboca Berry!")) % user->getDisplayName()).str());
                    }
                    eatBerry();
                }
                else if (berry == "ROWAPBERRY" && category == MoveDef::SPECIAL)
                {
                    if (userBattler.abilitySuppressed || userBattler.abilityId != "MAGICGUARD")
                    {
                        user->decreaseHP(std::max<size_t>(1, user->getTotalHP() / (ripen ? 4 : 8)));
                        result.messages.push_back(
                            (boost::format(lc::translate("%1% was hurt by the Rowap Berry!")) % user->getDisplayName()).str());
                    }
                    eatBerry();
                }
                else if (berry == "KEEBERRY" && category == MoveDef::PHYSICAL)
                {
                    auto raised = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::Defense, ripen ? 2 : 1,
                                                  result.messages, false);
                    if (raised.changed)
                        eatBerry();
                    if (raised.ejected)
                        ctx.shouldSwitchTarget = true;
                }
                else if (berry == "MARANGABERRY" && category == MoveDef::SPECIAL)
                {
                    auto raised = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::SpecialDefense,
                                                  ripen ? 2 : 1, result.messages, false);
                    if (raised.changed)
                        eatBerry();
                    if (raised.ejected)
                        ctx.shouldSwitchTarget = true;
                }
                if (switchHolder)
                    ctx.shouldSwitchTarget = true;
                if (forceOut)
                {
                    ctx.shouldSwitchUser = true;
                    ctx.forcedSwitchUser = true;
                }
            }
            if (damage > 0 && targetBattler.illusionActive)
            {
                targetBattler.illusionActive = false;
                targetBattler.illusionDisguise.reset();
                result.messages.push_back(
                    (boost::format(lc::translate("%1%'s illusion wore off!")) % target->getDisplayName()).str());
            }
            if (targetBattler.rage && damage > 0)
            {
                auto raised = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::Attack, 1,
                                              result.messages, false);
                if (raised.ejected)
                    ctx.shouldSwitchTarget = true;
            }
            if (targetBattler.beakBlast && def->hasFlag(MoveDef::Flag::Contact))
            {
                bool leafGuard = !userBattler.abilitySuppressed && userBattler.abilityId == "LEAFGUARD"
                              && data.fieldWeather() == Map::Weather::HARSH_SUNLIGHT
                              && ItemUtils::activeHeldId(user, &userBattler) != "UTILITYUMBRELLA";
                if (!leafGuard && !abilityBlocksStatus(user, userBattler, Pkmn::StatusCondition::BURN))
                    applyStatusCondition(user, Pkmn::StatusCondition::BURN, userBattler.toxicCounter, false,
                                         &userBattler);
            }
            if (targetBattler.shellTrap && def->getCategory() == MoveDef::PHYSICAL && damage > 0 && !target->isKO()
                && callDepth == 0)
            {
                targetBattler.shellTrap = false;
                Move::MovePtr trapMove;
                for (auto const& known : target->getMoves())
                {
                    if (known && known->getDefinition()
                        && known->getDefinition()->getFunctionCode()
                               == MoveDef::FunctionCode::UsedAfterUserTakesPhysicalDamage)
                    {
                        trapMove = known;
                        break;
                    }
                }
                if (trapMove)
                {
                    if (playerIsUser)
                        data.encounterMove.reset();
                    else
                        data.playerMove.reset();
                    targetBattler.movedThisTurn = true;
                    auto fired = MoveResolver::resolve(data, !playerIsUser, trapMove, callDepth + 1);
                    result.messages.insert(result.messages.end(), fired.messages.begin(), fired.messages.end());
                    result.userFainted   = result.userFainted || fired.targetFainted;
                    result.targetFainted = result.targetFainted || fired.userFainted;
                    if (fired.shouldSwitchUser)
                        result.shouldSwitchTarget = true;
                    if (fired.shouldSwitchTarget)
                        result.shouldSwitchUser = true;
                    if (fired.forcedSwitchUser)
                        result.forcedSwitchTarget = true;
                    if (fired.forcedSwitchTarget)
                        result.forcedSwitchUser = true;
                    if (user->isKO())
                        break;
                }
            }
            bool damp = (!userBattler.abilitySuppressed && userBattler.abilityId == "DAMP")
                     || (!targetBattler.abilitySuppressed && targetBattler.abilityId == "DAMP");
            bool attackerMagicGuard = !userBattler.abilitySuppressed && userBattler.abilityId == "MAGICGUARD";
            if (target->isKO() && !targetBattler.abilitySuppressed && def->hasFlag(MoveDef::Flag::Contact)
                && targetBattler.abilityId == "AFTERMATH" && !damp && !attackerMagicGuard)
                user->decreaseHP(std::max<size_t>(1, user->getTotalHP() / 4));
            if (target->isKO() && !targetBattler.abilitySuppressed && targetBattler.abilityId == "INNARDSOUT"
                && !attackerMagicGuard)
                // Gen 7+: Innards Out deals the HP lost on the KO hit, not overkill damage.
                user->decreaseHP(std::max<size_t>(1, hpBefore));
            if (target->isKO())
                abilityOnTargetFaint(data, playerIsUser, result.messages);
            if (target->isKO() && targetBattler.grudge)
                move->setCurrentPP(0);
            totalDamage += damage;
            // Gen 7+: Rough Skin / Iron Barbs / Static / Rocky Helmet / etc. fire per contact hit.
            if (damage > 0 && contactMove && !user->isKO())
            {
                bool sheerForceHit = !userBattler.abilitySuppressed && userBattler.abilityId == "SHEERFORCE"
                                  && def->getEffectChance() > 0;
                if (ItemUtils::applyContactItemEffects(user, target, true, true, damage, result.messages,
                                                       &userBattler, &targetBattler, sheerForceHit, true))
                    result.userFainted = true;
                {
                    AbilityEvent event;
                    event.playerSide = playerIsUser;
                    event.contact    = true;
                    event.moveType   = landedType.empty() ? def->getType() : landedType;
                    event.moveId     = def->getId();
                    triggerAbility(data, AbilityMoment::OnHit, event);
                    result.messages.insert(result.messages.end(), event.messages.begin(), event.messages.end());
                    if (event.userFainted)
                        result.userFainted = true;
                }
                if (user->isKO())
                    break;
            }
        }

        maybeThawFromFireHit(target, def);

        if (target->isKO())
        {
            if (targetBattler.endure && target->getHP() == 0)
            {
                target->setHP(1);
                result.messages.push_back(
                    (boost::format(lc::translate("%1% endured the hit!")) % target->getDisplayName()).str());
                targetBattler.endure = false;
            }
            else
            {
                result.messages.push_back(
                    (boost::format(lc::translate("Hit %1% time(s)!")) % (hit + 1)).str());
                break;
            }
        }
    }

    userBattler.attackOverride = savedAttackOverride;
    if (chargedThisMove)
        userBattler.chargeBoost = false;

    if (hits > 1 && !target->isKO() && !immune)
        result.messages.push_back((boost::format(lc::translate("Hit %1% time(s)!")) % hits).str());

    if (immune)
    {
        result.messages.push_back(
            (boost::format(lc::translate("It doesn't affect %1%...")) % target->getDisplayName()).str());
        crashOnFail();
        halfHpCostOnFail();
        selfKoOnFail();
    }
    else if (totalDamage > 0)
    {
        if (anyCritical)
            result.messages.push_back(lc::translate("A critical hit!"));
        if (effectiveness > 1.f)
            result.messages.push_back(lc::translate("It's super effective!"));
        else if (effectiveness < 1.f)
            result.messages.push_back(lc::translate("It's not very effective..."));
    }

    result.damageDealt   = totalDamage;
    result.critical      = anyCritical;
    result.effectiveness = effectiveness;
    result.hitPlayerSide = target == data.playerPkmn;

    ctx.damageDealt = totalDamage;
    ctx.immune      = immune;
    // Immunity nullifies the whole hit — no secondaries / bind / phaze.
    if (!immune)
        applyMoveFunctionCode(ctx);
    armGulpMissile(ctx);
    result.messages.insert(result.messages.end(), ctx.messages.begin(), ctx.messages.end());
    honorSuctionCups(ctx);
    result.shouldSwitchUser   = ctx.shouldSwitchUser;
    result.shouldSwitchTarget = ctx.shouldSwitchTarget;
    result.forcedSwitchTarget = ctx.forcedSwitchTarget;
    result.forcedSwitchUser   = ctx.forcedSwitchUser;
    if (totalDamage > 0)
    {
        targetBattler.tookDamageThisTurn = true;
        targetBattler.lastDamage         = static_cast<int>(totalDamage);
        targetBattler.lastDamageCategory = def->getCategory();
        if (targetBattler.bideActive)
            targetBattler.bideDamage += static_cast<int>(totalDamage);
        // Non-contact on-hit abilities (Justified, Stamina, Magician, …) still run once after the move.
        if (!contactMove)
        {
            AbilityEvent event;
            event.playerSide = playerIsUser;
            event.contact    = false;
            event.moveType   = landedType.empty() ? def->getType() : landedType;
            event.moveId     = def->getId();
            triggerAbility(data, AbilityMoment::OnHit, event);
            result.messages.insert(result.messages.end(), event.messages.begin(), event.messages.end());
            if (event.userFainted)
                result.userFainted = true;
        }
    }
    // Life Orb / Shell Bell / Expert Belt count Substitute damage. Contact retaliation stays on real hits.
    size_t itemDamage = totalDamage + subDamage;
    if (itemDamage > 0)
    {
        bool sheerForce = !userBattler.abilitySuppressed && userBattler.abilityId == "SHEERFORCE"
                       && def->getEffectChance() > 0;
        if (ItemUtils::applyContactItemEffects(user, target, false, true, itemDamage, result.messages, &userBattler,
                                               &targetBattler, sheerForce))
            result.userFainted = true;
    }
    noteMove(immune || !result.hit);

    result.targetFainted      = target->isKO();
    result.userFainted        = user->isKO();

    if (result.hit && target && target->isKO() && targetBattler.destinyBond && user && !user->isKO())
    {
        targetBattler.destinyBond = false;
        user->setHP(0);
        result.userFainted = true;
        result.messages.push_back(lc::translate("It took its attacker down with it!"));
    }

    if (callDepth == 0 && result.hit && def->hasFlag(MoveDef::Flag::Dance))
    {
        BattleBattler& dancerBattler = playerIsUser ? data.opponentBattler : data.playerBattler;
        auto           dancer        = playerIsUser ? data.opponentPkmn : data.playerPkmn;
        if (dancer && !dancer->isKO() && user && !user->isKO() && !dancerBattler.abilitySuppressed
            && dancerBattler.abilityId == "DANCER")
        {
            auto danced = std::make_shared<Move>(def);
            danced->setCurrentPP(1);
            result.messages.push_back(
                (boost::format(lc::translate("%1% joined the dance!")) % dancer->getDisplayName()).str());
            auto extra = MoveResolver::resolve(data, !playerIsUser, danced, callDepth + 1);
            result.messages.insert(result.messages.end(), extra.messages.begin(), extra.messages.end());
            if (extra.targetFainted)
                result.userFainted = true;
            if (extra.userFainted)
                result.targetFainted = true;
        }
    }

    refreshForme(data, true);
    refreshForme(data, false);
    applyImmediateHeldItems(data, result.messages);
    return result;
}
