#include "moveeffects.h"

#include "confusionstatus.h"
#include "map.h"
#include "singlebattlescene.h"
#include "statuscondition.h"
#include "utils.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <cmath>

namespace lc = boost::locale;

namespace
{
using SS = BattleBattler::StageStat;

void msg(MoveEffectContext& ctx, std::string const& text)
{
    ctx.messages.push_back(text);
}

void msgFmt(MoveEffectContext& ctx, boost::format const& f)
{
    ctx.messages.push_back(f.str());
}

bool changeStage(MoveEffectContext& ctx, BattleBattler& battler, Pkmn::PkmnPtr const& pkmn, SS stat, int delta,
                 bool checkMist)
{
    if (checkMist && delta < 0 && battler.mist)
    {
        msgFmt(ctx, boost::format(lc::translate("%1% is protected by Mist!")) % pkmn->getDisplayName());
        return false;
    }
    if (!battler.changeStage(stat, delta))
    {
        if (delta > 0)
            msgFmt(ctx, boost::format(lc::translate("%1%'s stats won't go any higher!")) % pkmn->getDisplayName());
        else
            msgFmt(ctx, boost::format(lc::translate("%1%'s stats won't go any lower!")) % pkmn->getDisplayName());
        return false;
    }
    char const* name = "stats";
    switch (stat)
    {
    case SS::Attack:
        name = "Attack";
        break;
    case SS::Defense:
        name = "Defense";
        break;
    case SS::SpecialAttack:
        name = "Sp. Atk";
        break;
    case SS::SpecialDefense:
        name = "Sp. Def";
        break;
    case SS::Speed:
        name = "Speed";
        break;
    case SS::Accuracy:
        name = "accuracy";
        break;
    case SS::Evasion:
        name = "evasiveness";
        break;
    default:
        break;
    }
    std::string how;
    if (delta >= 3)
        how = lc::translate("rose drastically");
    else if (delta == 2)
        how = lc::translate("sharply rose");
    else if (delta == 1)
        how = lc::translate("rose");
    else if (delta == -1)
        how = lc::translate("fell");
    else if (delta == -2)
        how = lc::translate("harshly fell");
    else
        how = lc::translate("severely fell");
    msgFmt(ctx, boost::format(lc::translate("%1%'s %2% %3%!")) % pkmn->getDisplayName() % name % how);
    return true;
}

bool rollSecondary(MoveEffectContext& ctx)
{
    size_t chance = ctx.def->getEffectChance();
    if (chance == 0)
        chance = 100;
    return Utils::randuint(1, 100) <= chance;
}

bool tryStatus(MoveEffectContext& ctx, Pkmn::StatusCondition status)
{
    if (!applyStatusCondition(ctx.target, status, ctx.targetBattler.toxicCounter))
        return false;
    msg(ctx, statusInflictedMessage(ctx.target));
    return true;
}

void healUser(MoveEffectContext& ctx, size_t amount)
{
    if (ctx.userBattler.healBlockTurns > 0)
        return;
    size_t maxHp = ctx.user->getTotalHP();
    size_t hp    = ctx.user->getHP();
    size_t newHp = std::min(maxHp, hp + amount);
    if (newHp == hp)
        return;
    ctx.user->setHP(newHp);
    msgFmt(ctx, boost::format(lc::translate("%1% had its HP restored.")) % ctx.user->getDisplayName());
}

size_t maxHpOf(Pkmn::PkmnPtr const& p)
{
    return std::max<size_t>(1, p->getTotalHP());
}
} // namespace

int moveHitCount(MoveDef::MoveDefPtr const& def)
{
    if (!def)
        return 1;
    switch (def->getFunctionCode())
    {
    case MoveDef::FunctionCode::HitTwoTimes:
    case MoveDef::FunctionCode::HitTwoTimesFlinchTarget:
    case MoveDef::FunctionCode::HitTwoTimesPoisonTarget:
    case MoveDef::FunctionCode::HitTwoTimesTargetThenTargetAlly:
        return 2;
    case MoveDef::FunctionCode::HitThreeTimesAlwaysCriticalHit:
    case MoveDef::FunctionCode::HitThreeTimesPowersUpWithEachHit:
        return 3;
    case MoveDef::FunctionCode::HitTwoToFiveTimes:
    case MoveDef::FunctionCode::HitTwoToFiveTimesOrThreeForAshGreninja:
    case MoveDef::FunctionCode::HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1:
    {
        // Gen III distribution: 2: 3/8, 3: 3/8, 4: 1/8, 5: 1/8
        size_t r = Utils::randuint(0, 7);
        if (r < 3)
            return 2;
        if (r < 6)
            return 3;
        if (r < 7)
            return 4;
        return 5;
    }
    default:
        return 1;
    }
}

bool isTwoTurnChargingMove(MoveDef::FunctionCode code)
{
    switch (code)
    {
    case MoveDef::FunctionCode::TwoTurnAttack:
    case MoveDef::FunctionCode::TwoTurnAttackBurnTarget:
    case MoveDef::FunctionCode::TwoTurnAttackChargeRaiseUserDefense1:
    case MoveDef::FunctionCode::TwoTurnAttackChargeRaiseUserSpAtk1:
    case MoveDef::FunctionCode::TwoTurnAttackFlinchTarget:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSky:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyParalyzeTarget:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyTargetCannotAct:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableRemoveProtections:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableUnderground:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableUnderwater:
    case MoveDef::FunctionCode::TwoTurnAttackOneTurnInSun:
    case MoveDef::FunctionCode::TwoTurnAttackParalyzeTarget:
    case MoveDef::FunctionCode::TwoTurnAttackRaiseUserSpAtkSpDefSpd2:
        return true;
    default:
        return false;
    }
}

bool isRechargeMove(MoveDef::FunctionCode code)
{
    return code == MoveDef::FunctionCode::AttackAndSkipNextTurn;
}

size_t moveEffectPower(MoveEffectContext& ctx, size_t basePower)
{
    auto code = ctx.def->getFunctionCode();
    size_t power = basePower;

    auto positiveStages = [](BattleBattler const& b) {
        int n = 0;
        for (size_t i = 0; i < BattleBattler::StageStatCount; ++i)
            if (b.stages[i] > 0)
                n += b.stages[i];
        return n;
    };

    switch (code)
    {
    case MoveDef::FunctionCode::DoublePowerIfTargetActed:
    case MoveDef::FunctionCode::DoublePowerIfTargetNotActed:
        break; // turn-order tracking not available; Gen III best-effort leave base
    case MoveDef::FunctionCode::DoublePowerIfTargetHPLessThanHalf:
        if (ctx.target && ctx.target->getHP() * 2 <= ctx.target->getTotalHP())
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetStatusProblem:
        if (ctx.target && ctx.target->getStatusCondition() != Pkmn::StatusCondition::NONE)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetPoisoned:
        if (ctx.target
            && (ctx.target->getStatusCondition() == Pkmn::StatusCondition::POISON
                || ctx.target->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON))
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetAsleepCureTarget:
        if (ctx.target && ctx.target->getStatusCondition() == Pkmn::StatusCondition::SLEEP)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetParalyzedCureTarget:
        if (ctx.target && ctx.target->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfUserPoisonedBurnedParalyzed:
        if (ctx.user->getStatusCondition() == Pkmn::StatusCondition::POISON
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::BURN
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfUserHasNoItem:
        if (!ctx.user->getHeldItem())
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetUnderground:
    case MoveDef::FunctionCode::DoublePowerIfTargetUnderwater:
    case MoveDef::FunctionCode::DoublePowerIfTargetInSky:
    case MoveDef::FunctionCode::RandomPowerDoublePowerIfTargetUnderground:
    case MoveDef::FunctionCode::FlinchTargetDoublePowerIfTargetInSky:
    case MoveDef::FunctionCode::BindTargetDoublePowerIfTargetUnderwater:
        if (ctx.targetBattler.semiInvulnerable)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerInElectricTerrain:
        if (ctx.data.terrain == SingleBattleSceneData::Terrain::Electric)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerAfterFusionBolt:
    case MoveDef::FunctionCode::DoublePowerAfterFusionFlare:
    case MoveDef::FunctionCode::DoublePowerIfAllyFaintedLastTurn:
    case MoveDef::FunctionCode::DoublePowerIfTargetLostHPThisTurn:
    case MoveDef::FunctionCode::DoublePowerIfUserLastMoveFailed:
    case MoveDef::FunctionCode::DoublePowerIfUserLostHPThisTurn:
    case MoveDef::FunctionCode::DoublePowerIfUserStatsLoweredThisTurn:
        break; // requires cross-turn battle history; leave base (Gen III-compatible no-op boost)
    case MoveDef::FunctionCode::PowerHigherWithUserHP:
        power = std::max<size_t>(1, basePower * ctx.user->getHP() / maxHpOf(ctx.user));
        break;
    case MoveDef::FunctionCode::PowerLowerWithUserHP:
        power = std::max<size_t>(1, basePower * (maxHpOf(ctx.user) - ctx.user->getHP() + 1) / maxHpOf(ctx.user));
        break;
    case MoveDef::FunctionCode::PowerHigherWithTargetHP:
        if (ctx.target)
            power = std::max<size_t>(1, basePower * ctx.target->getHP() / maxHpOf(ctx.target));
        break;
    case MoveDef::FunctionCode::PowerHigherWithUserPositiveStatStages:
        power = std::max<size_t>(1, 20 * (1 + positiveStages(ctx.userBattler)));
        break;
    case MoveDef::FunctionCode::PowerHigherWithTargetPositiveStatStages:
        power = std::max<size_t>(1, 20 * (1 + positiveStages(ctx.targetBattler)));
        break;
    case MoveDef::FunctionCode::PowerDependsOnUserStockpile:
        power = std::max<size_t>(1, 100 * std::max(0, ctx.userBattler.stockpile));
        break;
    case MoveDef::FunctionCode::PowerHigherWithLessPP:
    {
        size_t pp = ctx.move ? ctx.move->getCurrentPP() : 0;
        // Gen-style: more power as PP depletes (approx)
        static size_t const table[] = {40, 50, 60, 80, 100, 150, 200};
        power = table[std::min<size_t>(pp, 6)];
        break;
    }
    case MoveDef::FunctionCode::PowerHigherWithUserFasterThanTarget:
        if (ctx.target
            && battleSpeed(ctx.user, &ctx.userBattler) > battleSpeed(ctx.target, &ctx.targetBattler))
            power = std::max<size_t>(1, basePower * 2); // best-effort; real formula uses speed ratio
        break;
    case MoveDef::FunctionCode::PowerHigherWithTargetFasterThanUser:
        if (ctx.target
            && battleSpeed(ctx.target, &ctx.targetBattler) > battleSpeed(ctx.user, &ctx.userBattler))
            power = std::max<size_t>(1, basePower * 2);
        break;
    case MoveDef::FunctionCode::LowerTargetDefense1PowersUpInGravity:
        if (ctx.data.gravity)
            power = std::max<size_t>(1, basePower * 3 / 2);
        break;
    case MoveDef::FunctionCode::AlwaysCriticalHit:
        break;
    default:
        break;
    }

    if (ctx.userBattler.furyCutterPower > 0
        && code == MoveDef::FunctionCode::PowerHigherWithConsecutiveUse)
        power = std::min<size_t>(160, static_cast<size_t>(ctx.userBattler.furyCutterPower));

    return std::max<size_t>(1, power);
}

bool applyMoveFunctionCode(MoveEffectContext& ctx)
{
    using FC = MoveDef::FunctionCode;
    auto code = ctx.def->getFunctionCode();

    // Rest
    if (code == FC::HealUserFullyAndFallAsleep)
    {
        ctx.user->resetHP();
        ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
        applyStatusCondition(ctx.user, Pkmn::StatusCondition::SLEEP, ctx.userBattler.toxicCounter);
        ctx.user->setStatusTurns(3);
        msgFmt(ctx, boost::format(lc::translate("%1% went to sleep and became healthy!")) % ctx.user->getDisplayName());
        return true;
    }

    // Two-turn charging first turn
    if (isTwoTurnChargingMove(code) && !ctx.userBattler.charging)
    {
        ctx.userBattler.charging = true;
        if (code == FC::TwoTurnAttackInvulnerableInSky
            || code == FC::TwoTurnAttackInvulnerableInSkyParalyzeTarget
            || code == FC::TwoTurnAttackInvulnerableInSkyTargetCannotAct)
            ctx.userBattler.semiInvulnerable = true;
        if (code == FC::TwoTurnAttackInvulnerableUnderground
            || code == FC::TwoTurnAttackInvulnerableUnderwater)
            ctx.userBattler.semiInvulnerable = true;
        msgFmt(ctx, boost::format(lc::translate("%1% is glowing!")) % ctx.user->getDisplayName());
        return true; // charging — no hit this turn (resolver must skip damage)
    }
    if (ctx.userBattler.charging)
    {
        ctx.userBattler.charging = false;
        ctx.userBattler.semiInvulnerable = false;
    }

    if (isRechargeMove(code) && ctx.moveHit)
        ctx.userBattler.recharging = true;

    switch (code)
    {
    case FC::None:
        return true;

    // --- Stat raises (user) ---
    case FC::RaiseUserAttack1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        return true;
    case FC::RaiseUserAttack2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 2, false);
        return true;
    case FC::RaiseUserDefense1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        return true;
    case FC::RaiseUserDefense2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 2, false);
        return true;
    case FC::RaiseUserDefense3:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 3, false);
        return true;
    case FC::RaiseUserSpeed1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserSpeed2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        return true;
    case FC::RaiseUserSpAtk1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        return true;
    case FC::RaiseUserSpAtk2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 2, false);
        return true;
    case FC::RaiseUserSpAtk3:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 3, false);
        return true;
    case FC::RaiseUserSpDef2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 2, false);
        return true;
    case FC::RaiseUserEvasion1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Evasion, 1, false);
        return true;
    case FC::RaiseUserCriticalHitRate2:
        ctx.userBattler.focusEnergy = true;
        msgFmt(ctx, boost::format(lc::translate("%1% is getting pumped!")) % ctx.user->getDisplayName());
        return true;
    case FC::LowerTargetAttack1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        return true;
    case FC::LowerTargetAttack2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -2, true);
        return true;
    case FC::LowerTargetDefense1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Defense, -1, true);
        return true;
    case FC::LowerTargetDefense2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Defense, -2, true);
        return true;
    case FC::LowerTargetSpeed1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        return true;
    case FC::LowerTargetSpeed2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -2, true);
        return true;
    case FC::LowerTargetSpAtk1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -1, true);
        return true;
    case FC::LowerTargetSpAtk2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -2, true);
        return true;
    case FC::LowerTargetSpDef1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialDefense, -1, true);
        return true;
    case FC::LowerTargetSpDef2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialDefense, -2, true);
        return true;
    case FC::LowerTargetAccuracy1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Accuracy, -1, true);
        return true;
    case FC::LowerTargetEvasion2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Evasion, -2, true);
        return true;
    case FC::RaiseTargetAttack1:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, 1, true);
        return true;
    case FC::RaiseTargetSpDef1:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialDefense, 1, true);
        return true;

    case FC::RaiseUserAtkDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        return true;
    case FC::RaiseUserAtkSpAtk1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        return true;
    case FC::RaiseUserAtkSpd1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserAtkAcc1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Accuracy, 1, false);
        return true;
    case FC::RaiseUserAtk1Spd2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        return true;
    case FC::RaiseUserDefSpDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        return true;
    case FC::RaiseUserSpAtkSpDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        return true;
    case FC::RaiseUserSpAtkSpDefSpd1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserMainStats1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserMainStats1LoseThirdOfTotalHP:
        ctx.user->decreaseHP(std::max<size_t>(1, maxHpOf(ctx.user) / 3));
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserDefense1CurlUpUser:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        return true;
    case FC::RaiseUserEvasion2MinimizeUser:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Evasion, 2, false);
        ctx.userBattler.minimized = true;
        return true;
    case FC::RaiseUserSpeed2LowerUserWeight:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        return true;
    case FC::RaiseUserAtkDefAcc1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Accuracy, 1, false);
        return true;
    case FC::LowerUserAtkDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        return true;
    case FC::LowerUserDefense1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        return true;
    case FC::LowerUserDefSpDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, -1, false);
        return true;
    case FC::LowerUserSpAtk2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, -2, false);
        return true;
    case FC::LowerUserSpeed1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, -1, false);
        return true;
    case FC::LowerUserDefSpDefSpd1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, -1, false);
        return true;
    case FC::LowerUserDefSpDef1RaiseUserAtkSpAtkSpd2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 2, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 2, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        return true;
    case FC::LowerTargetAtkDef1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Defense, -1, true);
        return true;
    case FC::LowerTargetAtkSpAtk1:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -1, true);
        return true;
    case FC::LowerTargetAtkSpAtk1SwitchOutUser:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -1, true);
        ctx.shouldSwitchUser = true;
        return true;
    case FC::LowerTargetAttack1BypassSubstitute:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        return true;
    case FC::LowerTargetEvasion1RemoveSideEffects:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Evasion, -1, true);
        if (ctx.playerIsUser)
        {
            ctx.data.opponentSpikes = 0;
            ctx.data.opponentToxicSpikes = 0;
            ctx.data.opponentStealthRock = false;
            ctx.data.opponentStickyWeb = false;
        }
        else
        {
            ctx.data.playerSpikes = 0;
            ctx.data.playerToxicSpikes = 0;
            ctx.data.playerStealthRock = false;
            ctx.data.playerStickyWeb = false;
        }
        return true;
    case FC::ResetTargetStatStages:
        ctx.targetBattler.resetStages();
        msgFmt(ctx, boost::format(lc::translate("%1%'s stat changes were removed!")) % ctx.target->getDisplayName());
        return true;
    case FC::ResetAllBattlersStatStages:
        ctx.data.playerBattler.resetStages();
        ctx.data.opponentBattler.resetStages();
        msg(ctx, lc::translate("All stat changes were eliminated!"));
        return true;
    case FC::InvertTargetStatStages:
        ctx.targetBattler.invertStages();
        msgFmt(ctx, boost::format(lc::translate("%1%'s stat changes were inverted!")) % ctx.target->getDisplayName());
        return true;
    case FC::UserCopyTargetStatStages:
        ctx.userBattler.copyStagesFrom(ctx.targetBattler);
        msgFmt(ctx, boost::format(lc::translate("%1% copied %2%'s stat changes!")) % ctx.user->getDisplayName()
                         % ctx.target->getDisplayName());
        return true;
    case FC::UserTargetSwapStatStages:
        ctx.userBattler.swapStagesWith(ctx.targetBattler);
        msg(ctx, lc::translate("The battlers swapped stat changes!"));
        return true;
    case FC::UserTargetSwapAtkSpAtkStages:
    {
        std::swap(ctx.userBattler.stages[static_cast<size_t>(SS::Attack)],
                  ctx.targetBattler.stages[static_cast<size_t>(SS::Attack)]);
        std::swap(ctx.userBattler.stages[static_cast<size_t>(SS::SpecialAttack)],
                  ctx.targetBattler.stages[static_cast<size_t>(SS::SpecialAttack)]);
        msg(ctx, lc::translate("The battlers swapped their Attack and Sp. Atk changes!"));
        return true;
    }
    case FC::UserTargetSwapDefSpDefStages:
    {
        std::swap(ctx.userBattler.stages[static_cast<size_t>(SS::Defense)],
                  ctx.targetBattler.stages[static_cast<size_t>(SS::Defense)]);
        std::swap(ctx.userBattler.stages[static_cast<size_t>(SS::SpecialDefense)],
                  ctx.targetBattler.stages[static_cast<size_t>(SS::SpecialDefense)]);
        msg(ctx, lc::translate("The battlers swapped their Defense and Sp. Def changes!"));
        return true;
    }
    case FC::RaiseTargetAttack2ConfuseTarget:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, 2, false);
        {
            ConfusionStatus& conf = ctx.playerIsUser ? ctx.data.opponentConfusion : ctx.data.playerConfusion;
            if (conf.apply())
                msgFmt(ctx, boost::format(lc::translate("%1% became confused!")) % ctx.target->getDisplayName());
        }
        return true;
    case FC::RaiseTargetSpAtk1ConfuseTarget:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, 1, false);
        {
            ConfusionStatus& conf = ctx.playerIsUser ? ctx.data.opponentConfusion : ctx.data.playerConfusion;
            if (conf.apply())
                msgFmt(ctx, boost::format(lc::translate("%1% became confused!")) % ctx.target->getDisplayName());
        }
        return true;

    // --- Status infliction ---
    case FC::BurnTarget:
    case FC::BurnTargetIfTargetStatsRaisedThisTurn:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::BURN);
        return true;
    case FC::BurnFlinchTarget:
        if (rollSecondary(ctx))
        {
            tryStatus(ctx, Pkmn::StatusCondition::BURN);
            ctx.targetBattler.flinched = true;
        }
        return true;
    case FC::FreezeTarget:
    case FC::FreezeTargetAlwaysHitsInHail:
    case FC::FreezeTargetSuperEffectiveAgainstWater:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::FREEZE);
        return true;
    case FC::FreezeFlinchTarget:
        if (rollSecondary(ctx))
        {
            tryStatus(ctx, Pkmn::StatusCondition::FREEZE);
            ctx.targetBattler.flinched = true;
        }
        return true;
    case FC::ParalyzeTarget:
    case FC::ParalyzeTargetAlwaysHitsInRainHitsTargetInSky:
    case FC::ParalyzeTargetIfNotTypeImmune:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
        return true;
    case FC::ParalyzeFlinchTarget:
        if (rollSecondary(ctx))
        {
            tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
            ctx.targetBattler.flinched = true;
        }
        return true;
    case FC::PoisonTarget:
    case FC::PoisonTargetLowerTargetSpeed1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::POISON);
        if (code == FC::PoisonTargetLowerTargetSpeed1)
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        return true;
    case FC::HitTwoTimesPoisonTarget:
        if (rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::POISON);
        return true;
    case FC::BadPoisonTarget:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::BADLY_POISON);
        return true;
    case FC::SleepTarget:
    case FC::SleepTargetIfUserDarkrai:
    case FC::SleepTargetChangeUserMeloettaForm:
        tryStatus(ctx, Pkmn::StatusCondition::SLEEP);
        return true;
    case FC::SleepTargetNextTurn:
        ctx.targetBattler.yawnPending = true;
        msgFmt(ctx, boost::format(lc::translate("%1% grew drowsy!")) % ctx.target->getDisplayName());
        return true;
    case FC::ParalyzeBurnOrFreezeTarget:
        if (!rollSecondary(ctx)) return true;
        switch (Utils::randuint(0, 2))
        {
        case 0: tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS); break;
        case 1: tryStatus(ctx, Pkmn::StatusCondition::BURN); break;
        default: tryStatus(ctx, Pkmn::StatusCondition::FREEZE); break;
        }
        return true;
    case FC::ConfuseTarget:
    case FC::ConfuseTargetAlwaysHitsInRainHitsTargetInSky:
    {
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        ConfusionStatus& conf = ctx.playerIsUser ? ctx.data.opponentConfusion : ctx.data.playerConfusion;
        if (conf.apply())
            msgFmt(ctx, boost::format(lc::translate("%1% became confused!")) % ctx.target->getDisplayName());
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    case FC::AttractTarget:
        msgFmt(ctx, boost::format(lc::translate("%1% fell in love!")) % ctx.target->getDisplayName());
        return true;

    // --- Flinch / Protect ---
    case FC::FlinchTarget:
    case FC::FlinchTargetDoublePowerIfTargetInSky:
    case FC::FlinchTargetFailsIfNotUserFirstTurn:
    case FC::FlinchTargetFailsIfUserNotAsleep:
    case FC::HitTwoTimesFlinchTarget:
    case FC::TwoTurnAttackFlinchTarget:
        if (rollSecondary(ctx) || ctx.def->getEffectChance() == 0 || ctx.def->getEffectChance() == 100)
            ctx.targetBattler.flinched = true;
        return true;
    case FC::ProtectUser:
    case FC::ProtectUserBanefulBunker:
    case FC::ProtectUserFromDamagingMovesKingsShield:
    case FC::ProtectUserFromDamagingMovesObstruct:
    case FC::ProtectUserFromTargetingMovesSpikyShield:
        ctx.userBattler.protect = true;
        msgFmt(ctx, boost::format(lc::translate("%1% protected itself!")) % ctx.user->getDisplayName());
        return true;
    case FC::UserEnduresFaintingThisTurn:
        ctx.userBattler.endure = true;
        msgFmt(ctx, boost::format(lc::translate("%1% braced itself!")) % ctx.user->getDisplayName());
        return true;
    case FC::UserMakeSubstitute:
    {
        size_t cost = std::max<size_t>(1, maxHpOf(ctx.user) / 4);
        if (ctx.user->getHP() <= cost || ctx.userBattler.substituteHP > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.user->decreaseHP(cost);
        ctx.userBattler.substituteHP = static_cast<int>(cost);
        msgFmt(ctx, boost::format(lc::translate("%1% put up a substitute!")) % ctx.user->getDisplayName());
        return true;
    }

    // --- Healing ---
    case FC::HealUserHalfOfTotalHP:
    case FC::HealUserHalfOfTotalHPLoseFlyingTypeThisTurn:
        healUser(ctx, maxHpOf(ctx.user) / 2);
        return true;
    case FC::HealUserDependingOnWeather:
    {
        size_t amount = maxHpOf(ctx.user) / 2;
        if (ctx.data.weather == Map::Weather::HARSH_SUNLIGHT)
            amount = maxHpOf(ctx.user) * 2 / 3;
        else if (ctx.data.weather == Map::Weather::RAIN || ctx.data.weather == Map::Weather::SANDSTORM
                 || ctx.data.weather == Map::Weather::HAIL)
            amount = maxHpOf(ctx.user) / 4;
        healUser(ctx, amount);
        return true;
    }
    case FC::HealUserByHalfOfDamageDone:
    case FC::HealUserByHalfOfDamageDoneIfTargetAsleep:
    case FC::HealUserByThreeQuartersOfDamageDone:
    {
        size_t amount = ctx.damageDealt / 2;
        if (code == FC::HealUserByThreeQuartersOfDamageDone)
            amount = ctx.damageDealt * 3 / 4;
        healUser(ctx, std::max<size_t>(1, amount));
        return true;
    }
    case FC::HealTargetHalfOfTotalHP:
    {
        size_t amount = maxHpOf(ctx.target) / 2;
        size_t newHp  = std::min(maxHpOf(ctx.target), ctx.target->getHP() + amount);
        ctx.target->setHP(newHp);
        msgFmt(ctx, boost::format(lc::translate("%1% had its HP restored.")) % ctx.target->getDisplayName());
        return true;
    }
    case FC::CureUserBurnPoisonParalysis:
        if (ctx.user->getStatusCondition() == Pkmn::StatusCondition::BURN
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::POISON
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
        {
            ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
            msgFmt(ctx, boost::format(lc::translate("%1%'s status returned to normal!")) % ctx.user->getDisplayName());
        }
        return true;
    case FC::CureTargetBurn:
        if (ctx.target->getStatusCondition() == Pkmn::StatusCondition::BURN)
        {
            ctx.target->setStatusCondition(Pkmn::StatusCondition::NONE);
            msgFmt(ctx, boost::format(lc::translate("%1%'s burn was healed!")) % ctx.target->getDisplayName());
        }
        return true;
    case FC::CureUserPartyStatus:
        ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
        msg(ctx, lc::translate("A soothing aroma wafted through the area!"));
        return true;

    // --- Recoil / OHKO / fixed damage ---
    case FC::RecoilQuarterOfDamageDealt:
        if (ctx.damageDealt > 0)
            ctx.user->decreaseHP(std::max<size_t>(1, ctx.damageDealt / 4));
        return true;
    case FC::RecoilThirdOfDamageDealt:
    case FC::RecoilThirdOfDamageDealtBurnTarget:
    case FC::RecoilThirdOfDamageDealtParalyzeTarget:
        if (ctx.damageDealt > 0)
            ctx.user->decreaseHP(std::max<size_t>(1, ctx.damageDealt / 3));
        if (code == FC::RecoilThirdOfDamageDealtBurnTarget && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::BURN);
        if (code == FC::RecoilThirdOfDamageDealtParalyzeTarget && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
        return true;
    case FC::RecoilHalfOfDamageDealt:
        if (ctx.damageDealt > 0)
            ctx.user->decreaseHP(std::max<size_t>(1, ctx.damageDealt / 2));
        return true;
    case FC::OHKO:
    case FC::OHKOIce:
    case FC::OHKOHitsUndergroundTarget:
        if (ctx.user->getLevel() >= ctx.target->getLevel())
        {
            ctx.target->setHP(0);
            msg(ctx, lc::translate("It's a one-hit KO!"));
        }
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    case FC::FixedDamage20:
        ctx.target->decreaseHP(20);
        return true;
    case FC::FixedDamage40:
        ctx.target->decreaseHP(40);
        return true;
    case FC::FixedDamageHalfTargetHP:
        ctx.target->decreaseHP(std::max<size_t>(1, ctx.target->getHP() / 2));
        return true;
    case FC::FixedDamageUserLevel:
        ctx.target->decreaseHP(ctx.user->getLevel());
        return true;
    case FC::FixedDamageUserLevelRandom:
        ctx.target->decreaseHP(Utils::randuint(1, ctx.user->getLevel()));
        return true;
    case FC::LowerTargetHPToUserHP:
        if (ctx.target->getHP() > ctx.user->getHP())
            ctx.target->setHP(ctx.user->getHP());
        return true;
    case FC::UserLosesHalfOfTotalHP:
    case FC::UserLosesHalfOfTotalHPExplosive:
        ctx.user->decreaseHP(maxHpOf(ctx.user) / 2);
        return true;
    case FC::UserFaintsExplosive:
    case FC::UserFaintsFixedDamageUserHP:
    case FC::UserFaintsLowerTargetAtkSpAtk2:
    case FC::UserFaintsPowersUpInMistyTerrainExplosive:
        ctx.user->setHP(0);
        if (code == FC::UserFaintsLowerTargetAtkSpAtk2)
        {
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -2, true);
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -2, true);
        }
        return true;
    case FC::CrashDamageIfFailsUnusableInGravity:
        // applied by resolver on miss
        return true;

    // --- Multi-turn / binding ---
    case FC::BindTarget:
    case FC::BindTargetDoublePowerIfTargetUnderwater:
    case FC::TrapTargetInBattle:
    case FC::TrapTargetInBattleMainEffect:
        ctx.targetBattler.bindingTurns = static_cast<int>(Utils::randuint(4, 5));
        ctx.targetBattler.trapped      = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was trapped!")) % ctx.target->getDisplayName());
        return true;
    case FC::MultiTurnAttackConfuseUserAtEnd:
        ctx.userBattler.multiTurnLock = static_cast<int>(Utils::randuint(2, 3));
        ctx.userBattler.rampageConfuseAtEnd = true;
        return true;
    case FC::MultiTurnAttackPowersUpEachTurn:
        ctx.userBattler.multiTurnLock = 2;
        return true;
    case FC::AttackAndSkipNextTurn:
        return true;
    case FC::StartLeechSeedTarget:
        if (hasType(ctx.target, "GRASS") || ctx.targetBattler.seeded)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.seeded = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was seeded!")) % ctx.target->getDisplayName());
        return true;

    // --- Hazards ---
    case FC::AddSpikesToFoeSide:
        if (ctx.playerIsUser)
            ctx.data.opponentSpikes = std::min(3, ctx.data.opponentSpikes + 1);
        else
            ctx.data.playerSpikes = std::min(3, ctx.data.playerSpikes + 1);
        msg(ctx, lc::translate("Spikes were scattered all around the opposing team's feet!"));
        return true;
    case FC::AddToxicSpikesToFoeSide:
        if (ctx.playerIsUser)
            ctx.data.opponentToxicSpikes = std::min(2, ctx.data.opponentToxicSpikes + 1);
        else
            ctx.data.playerToxicSpikes = std::min(2, ctx.data.playerToxicSpikes + 1);
        msg(ctx, lc::translate("Poison spikes were scattered all around the opposing team's feet!"));
        return true;
    case FC::AddStealthRocksToFoeSide:
        if (ctx.playerIsUser)
            ctx.data.opponentStealthRock = true;
        else
            ctx.data.playerStealthRock = true;
        msg(ctx, lc::translate("Pointed stones float in the air around the opposing team!"));
        return true;
    case FC::AddStickyWebToFoeSide:
        if (ctx.playerIsUser)
            ctx.data.opponentStickyWeb = true;
        else
            ctx.data.playerStickyWeb = true;
        msg(ctx, lc::translate("A sticky web has been laid out beneath the opposing team's feet!"));
        return true;
    case FC::RemoveUserBindingAndEntryHazards:
        ctx.userBattler.bindingTurns = 0;
        ctx.userBattler.trapped      = false;
        if (ctx.playerIsUser)
        {
            ctx.data.playerSpikes = ctx.data.playerToxicSpikes = 0;
            ctx.data.playerStealthRock = ctx.data.playerStickyWeb = false;
        }
        else
        {
            ctx.data.opponentSpikes = ctx.data.opponentToxicSpikes = 0;
            ctx.data.opponentStealthRock = ctx.data.opponentStickyWeb = false;
        }
        msg(ctx, lc::translate("The effects of hazards disappeared!"));
        return true;

    // --- Weather / field ---
    case FC::StartRainWeather:
        ctx.data.weather = Map::Weather::RAIN;
        msg(ctx, lc::translate("It started to rain!"));
        return true;
    case FC::StartSunWeather:
        ctx.data.weather = Map::Weather::HARSH_SUNLIGHT;
        msg(ctx, lc::translate("The sunlight turned harsh!"));
        return true;
    case FC::StartSandstormWeather:
        ctx.data.weather = Map::Weather::SANDSTORM;
        msg(ctx, lc::translate("A sandstorm kicked up!"));
        return true;
    case FC::StartHailWeather:
        ctx.data.weather = Map::Weather::HAIL;
        msg(ctx, lc::translate("It started to hail!"));
        return true;
    case FC::StartElectricTerrain:
        ctx.data.terrain = SingleBattleSceneData::Terrain::Electric;
        ctx.data.terrainTurns = 5;
        msg(ctx, lc::translate("An electric current ran across the battlefield!"));
        return true;
    case FC::StartGrassyTerrain:
        ctx.data.terrain = SingleBattleSceneData::Terrain::Grassy;
        ctx.data.terrainTurns = 5;
        msg(ctx, lc::translate("Grass grew to cover the battlefield!"));
        return true;
    case FC::StartMistyTerrain:
        ctx.data.terrain = SingleBattleSceneData::Terrain::Misty;
        ctx.data.terrainTurns = 5;
        msg(ctx, lc::translate("Mist swirled around the battlefield!"));
        return true;
    case FC::StartPsychicTerrain:
        ctx.data.terrain = SingleBattleSceneData::Terrain::Psychic;
        ctx.data.terrainTurns = 5;
        msg(ctx, lc::translate("The battlefield got weird!"));
        return true;
    case FC::RemoveTerrain:
        ctx.data.terrain = SingleBattleSceneData::Terrain::None;
        ctx.data.terrainTurns = 0;
        msg(ctx, lc::translate("The terrain returned to normal!"));
        return true;
    case FC::StartGravity:
        ctx.data.gravity = true;
        ctx.data.gravityTurns = 5;
        msg(ctx, lc::translate("Gravity intensified!"));
        return true;
    case FC::StartSlowerBattlersActFirst:
        ctx.data.trickRoom = !ctx.data.trickRoom;
        ctx.data.trickRoomTurns = 5;
        msg(ctx, ctx.data.trickRoom ? lc::translate("The dimensions were twisted!")
                                    : lc::translate("The twisted dimensions returned to normal!"));
        return true;

    // --- Switching ---
    case FC::SwitchOutUserDamagingMove:
    case FC::SwitchOutUserStatusMove:
    case FC::SwitchOutUserPassOnEffects:
        ctx.shouldSwitchUser = true;
        return true;
    case FC::SwitchOutTargetDamagingMove:
    case FC::SwitchOutTargetStatusMove:
        ctx.shouldSwitchTarget = true;
        ctx.forcedSwitchTarget = true;
        return true;

    // --- Screens / mist / safeguard ---
    case FC::StartWeakenPhysicalDamageAgainstUserSide:
        msg(ctx, lc::translate("Reflect raised the team's Defense!"));
        return true;
    case FC::StartWeakenSpecialDamageAgainstUserSide:
        msg(ctx, lc::translate("Light Screen raised the team's Sp. Def!"));
        return true;
    case FC::StartUserSideImmunityToStatStageLowering:
        ctx.userBattler.mist = true;
        msgFmt(ctx, boost::format(lc::translate("%1%'s team became shrouded in mist!")) % ctx.user->getDisplayName());
        return true;
    case FC::StartUserSideImmunityToInflictedStatus:
        ctx.userBattler.safeguard = true;
        msg(ctx, lc::translate("The team became cloaked in a mystical veil!"));
        return true;
    case FC::RemoveScreens:
        msg(ctx, lc::translate("The opposing team's screens disappeared!"));
        return true;
    case FC::StartPerishCountsForAllBattlers:
        if (ctx.data.playerBattler.perishCount < 0)
            ctx.data.playerBattler.perishCount = 3;
        if (ctx.data.opponentBattler.perishCount < 0)
            ctx.data.opponentBattler.perishCount = 3;
        msg(ctx, lc::translate("All Pokémon will faint in three turns!"));
        return true;
    case FC::StartHealUserEachTurn:
        msgFmt(ctx, boost::format(lc::translate("%1% planted its roots!")) % ctx.user->getDisplayName());
        return true;
    case FC::UserSwapBaseAtkDef:
        msgFmt(ctx, boost::format(lc::translate("%1% switched its Attack and Defense!")) % ctx.user->getDisplayName());
        return true;
    case FC::TransformUserIntoTarget:
        msgFmt(ctx, boost::format(lc::translate("%1% transformed into %2%!")) % ctx.user->getDisplayName()
                         % ctx.target->getDisplayName());
        return true;
    case FC::DisableTargetLastMoveUsed:
        ctx.targetBattler.disableTurns = 4;
        msgFmt(ctx, boost::format(lc::translate("%1%'s move was disabled!")) % ctx.target->getDisplayName());
        return true;
    case FC::DisableTargetUsingSameMoveConsecutively:
        ctx.targetBattler.torment = 1;
        msgFmt(ctx, boost::format(lc::translate("%1% was subjected to torment!")) % ctx.target->getDisplayName());
        return true;
    case FC::DisableTargetStatusMoves:
        ctx.targetBattler.tauntTurns = 3;
        msgFmt(ctx, boost::format(lc::translate("%1% fell for the taunt!")) % ctx.target->getDisplayName());
        return true;

    // Multi-hit shells — effects applied after hits by resolver
    case FC::HitTwoTimes:
    case FC::HitTwoToFiveTimes:
    case FC::HitTwoToFiveTimesOrThreeForAshGreninja:
    case FC::HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1:
    case FC::HitThreeTimesAlwaysCriticalHit:
    case FC::HitThreeTimesPowersUpWithEachHit:
    case FC::HitTwoTimesTargetThenTargetAlly:
    case FC::TwoTurnAttack:
    case FC::TwoTurnAttackBurnTarget:
    case FC::TwoTurnAttackChargeRaiseUserDefense1:
    case FC::TwoTurnAttackChargeRaiseUserSpAtk1:
    case FC::TwoTurnAttackInvulnerableInSky:
    case FC::TwoTurnAttackInvulnerableInSkyParalyzeTarget:
    case FC::TwoTurnAttackInvulnerableInSkyTargetCannotAct:
    case FC::TwoTurnAttackInvulnerableRemoveProtections:
    case FC::TwoTurnAttackInvulnerableUnderground:
    case FC::TwoTurnAttackInvulnerableUnderwater:
    case FC::TwoTurnAttackOneTurnInSun:
    case FC::TwoTurnAttackParalyzeTarget:
    case FC::TwoTurnAttackRaiseUserSpAtkSpDefSpd2:
        if (code == FC::TwoTurnAttackBurnTarget && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::BURN);
        if ((code == FC::TwoTurnAttackParalyzeTarget
             || code == FC::TwoTurnAttackInvulnerableInSkyParalyzeTarget)
            && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
        if (code == FC::HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1)
        {
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        }
        return true;

    case FC::RaiseUserAttack3IfTargetFaints:
        if (ctx.target->isKO())
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 3, false);
        return true;
    case FC::PowerHigherWithConsecutiveUse:
        if (ctx.userBattler.furyCutterPower == 0)
            ctx.userBattler.furyCutterPower = static_cast<int>(ctx.def->getPower());
        else
            ctx.userBattler.furyCutterPower = std::min(160, ctx.userBattler.furyCutterPower * 2);
        return true;


    case FC::RaiseTargetAtkSpAtk2:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, 2, false);
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, 2, false);
        return true;
    case FC::RaiseTargetRandomStat2:
    {
        auto stat = static_cast<SS>(Utils::randuint(0, static_cast<size_t>(SS::Evasion)));
        changeStage(ctx, ctx.targetBattler, ctx.target, stat, 2, false);
        return true;
    }
    case FC::RaiseUserAtkSpAtk1Or2InSun:
    {
        int d = (ctx.data.weather == Map::HARSH_SUNLIGHT) ? 2 : 1;
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, d, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, d, false);
        return true;
    }
    case FC::RaiseUserMainStats1TrapUserInBattle:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        ctx.userBattler.trapped = true;
        return true;
    case FC::RaiseUserSpDef1PowerUpElectricMove:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        return true;
    case FC::RaiseAlliesAtkDef1:
    case FC::RaiseGrassBattlersDef1:
    case FC::RaiseGroundedGrassBattlersAtkSpAtk1:
    case FC::RaisePlusMinusUserAndAlliesAtkSpAtk1:
    case FC::RaisePlusMinusUserAndAlliesDefSpDef1:
        // Singles: apply to user as best-effort Gen III-compatible ally effect
        if (code == FC::RaiseAlliesAtkDef1 || code == FC::RaisePlusMinusUserAndAlliesAtkSpAtk1
            || code == FC::RaiseGroundedGrassBattlersAtkSpAtk1)
        {
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
            changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        }
        if (code == FC::RaiseAlliesAtkDef1 || code == FC::RaiseGrassBattlersDef1
            || code == FC::RaisePlusMinusUserAndAlliesDefSpDef1)
        {
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
            if (code != FC::RaiseGrassBattlersDef1)
                changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        }
        return true;
    case FC::LowerPoisonedTargetAtkSpAtkSpd1:
        if (ctx.target
            && (ctx.target->getStatusCondition() == Pkmn::StatusCondition::POISON
                || ctx.target->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON))
        {
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -1, true);
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        }
        return true;
    case FC::LowerTargetSpAtk2IfCanAttract:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -2, true);
        return true;
    case FC::LowerTargetSpeed1MakeTargetWeakerToFire:
    case FC::LowerTargetSpeed1WeakerInGrassyTerrain:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        return true;
    case FC::LowerTargetDefense1PowersUpInGravity:
        if (rollSecondary(ctx) || ctx.def->getCategory() == MoveDef::STATUS)
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Defense, -1, true);
        return true;
    case FC::UserAddStockpileRaiseDefSpDef1:
        if (ctx.userBattler.stockpile < 3)
        {
            ++ctx.userBattler.stockpile;
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
            changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
            msgFmt(ctx, boost::format(lc::translate("%1% stockpiled %2%!")) % ctx.user->getDisplayName()
                            % ctx.userBattler.stockpile);
        }
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    case FC::HealUserDependingOnUserStockpile:
    {
        int s = ctx.userBattler.stockpile;
        if (s <= 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        size_t frac = (s == 1) ? 4 : (s == 2) ? 2 : 1;
        healUser(ctx, maxHpOf(ctx.user) / frac);
        ctx.userBattler.stockpile = 0;
        return true;
    }
    case FC::CurseTargetOrLowerUserSpd1RaiseUserAtkDef1:
        // Ghost curse omitted without type check helpers; non-Ghost path (Gen III)
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        return true;
    case FC::MaxUserAttackLoseHalfOfTotalHP:
        ctx.user->decreaseHP(std::max<size_t>(1, maxHpOf(ctx.user) / 2));
        while (ctx.userBattler.getStage(SS::Attack) < 6)
            ctx.userBattler.changeStage(SS::Attack, 1);
        msgFmt(ctx, boost::format(lc::translate("%1% cut its own HP and maximized Attack!"))
                        % ctx.user->getDisplayName());
        return true;
    case FC::EnsureNextCriticalHit:
        ctx.userBattler.focusEnergy = true;
        msgFmt(ctx, boost::format(lc::translate("%1% is getting pumped!")) % ctx.user->getDisplayName());
        return true;
    case FC::HealUserPositionNextTurn:
        // Wish: store heal amount on battler via stockpile misuse avoided — use bindingTurns as sentinel unused
        msgFmt(ctx, boost::format(lc::translate("%1% made a wish!")) % ctx.user->getDisplayName());
        return true;
    case FC::UserStealTargetPositiveStatStages:
        for (size_t i = 0; i < BattleBattler::StageStatCount; ++i)
        {
            int s = ctx.targetBattler.stages[i];
            if (s > 0)
            {
                ctx.userBattler.changeStage(static_cast<SS>(i), s);
                ctx.targetBattler.stages[i] = 0;
            }
        }
        msg(ctx, lc::translate("The stat changes were stolen!"));
        return true;
    case FC::GiveUserStatusToTarget:
        if (ctx.user->getStatusCondition() != Pkmn::StatusCondition::NONE
            && ctx.target->getStatusCondition() == Pkmn::StatusCondition::NONE)
        {
            tryStatus(ctx, ctx.user->getStatusCondition());
            ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
        }
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    case FC::CureTargetStatusHealUserHalfOfTotalHP:
        if (ctx.target && ctx.target->getStatusCondition() != Pkmn::StatusCondition::NONE)
        {
            ctx.target->setStatusCondition(Pkmn::StatusCondition::NONE);
            healUser(ctx, maxHpOf(ctx.user) / 2);
        }
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    case FC::HealUserDependingOnSandstorm:
    {
        size_t amount = maxHpOf(ctx.user) / 2;
        if (ctx.data.weather == Map::SANDSTORM)
            amount = maxHpOf(ctx.user) * 2 / 3;
        healUser(ctx, amount);
        return true;
    }
    case FC::HealUserAndAlliesQuarterOfTotalHP:
    case FC::HealUserAndAlliesQuarterOfTotalHPCureStatus:
        healUser(ctx, maxHpOf(ctx.user) / 4);
        if (code == FC::HealUserAndAlliesQuarterOfTotalHPCureStatus)
            ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
        return true;
    case FC::HealUserByTargetAttackLowerTargetAttack1:
    {
        size_t atk = ctx.target->getStats()[PkmnDef::ATTACK];
        healUser(ctx, std::max<size_t>(1, atk));
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        return true;
    }
    case FC::UserTargetSwapItems:
    {
        auto ui = ctx.user->getHeldItem();
        auto ti = ctx.target->getHeldItem();
        ctx.user->setHeldItem(ti);
        ctx.target->setHeldItem(ui);
        msg(ctx, lc::translate("The items were swapped!"));
        return true;
    }
    case FC::UserTakesTargetItem:
        if (ctx.target && ctx.target->getHeldItem() && !ctx.user->getHeldItem())
        {
            ctx.user->setHeldItem(ctx.target->getHeldItem());
            ctx.target->setHeldItem(nullptr);
            msg(ctx, lc::translate("The item was stolen!"));
        }
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    case FC::UserTargetAverageHP:
    {
        size_t avg = (ctx.user->getHP() + ctx.target->getHP()) / 2;
        ctx.user->setHP(std::min(avg, maxHpOf(ctx.user)));
        ctx.target->setHP(std::min(avg, maxHpOf(ctx.target)));
        msg(ctx, lc::translate("The battlers shared their HP!"));
        return true;
    }
    case FC::UserTargetSwapBaseSpeed:
        // Approximate with Speed stages swap
        {
            int us = ctx.userBattler.getStage(SS::Speed);
            int ts = ctx.targetBattler.getStage(SS::Speed);
            ctx.userBattler.stages[static_cast<size_t>(SS::Speed)] = ts;
            ctx.targetBattler.stages[static_cast<size_t>(SS::Speed)] = us;
            msg(ctx, lc::translate("The battlers swapped their Speed!"));
        }
        return true;
    case FC::FailsIfNotUserFirstTurn:
        // Without turn counter, allow (best-effort)
        return true;
    case FC::DoesNothingCongratulations:
        msg(ctx, lc::translate("Congratulations!"));
        return true;
    case FC::DoesNothingFailsIfNoAlly:
    case FC::DoesNothingUnusableInGravity:
        if (code == FC::DoesNothingUnusableInGravity && ctx.data.gravity)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        msg(ctx, lc::translate("But nothing happened!"));
        return true;

    // Remaining Essentials FunctionCodes: Gen III best-effort / documented no-op
    case FC::AddGhostTypeToTarget:
    case FC::AddGrassTypeToTarget:
    case FC::AddMoneyGainedFromBattle:
    case FC::AllBattlersConsumeBerry:
    case FC::AttackTwoTurnsLater:
    case FC::AttackerFaintsIfUserFaints:
    case FC::BounceBackProblemCausingStatusMoves:
    case FC::BurnAttackerBeforeUserActs:
    case FC::CannotBeRedirected:
    case FC::CannotMakeTargetFaint:
    case FC::CategoryDependsOnHigherDamageIgnoreTargetAbility:
    case FC::CategoryDependsOnHigherDamagePoisonTarget:
    case FC::CorrodeTargetItem:
    case FC::CounterDamagePlusHalf:
    case FC::CounterPhysicalDamage:
    case FC::CounterSpecialDamage:
    case FC::DamageTargetAlly:
    case FC::DestroyTargetBerryOrGem:
    case FC::DisableTargetHealingMoves:
    case FC::DisableTargetMovesKnownByUser:
    case FC::DisableTargetSoundMoves:
    case FC::DisableTargetUsingDifferentMove:
    case FC::DoubleMoneyGainedFromBattle:
    case FC::EffectDependsOnEnvironment:
    case FC::EffectivenessIncludesFlyingType:
    case FC::EnsureNextMoveAlwaysHits:
    case FC::FailsIfTargetActed:
    case FC::FailsIfTargetHasNoItem:
    case FC::FailsIfUserDamagedThisTurn:
    case FC::FailsIfUserHasUnusedMove:
    case FC::FailsIfUserNotConsumedBerry:
    case FC::FailsUnlessTargetSharesTypeWithUser:
    case FC::FirePledge:
    case FC::GrassPledge:
    case FC::HealAllyOrDamageFoe:
    case FC::HealTargetDependingOnGrassyTerrain:
    case FC::HigherPriorityInGrassyTerrain:
    case FC::HitOncePerUserTeamMember:
    case FC::HitsAllFoesAndPowersUpInPsychicTerrain:
    case FC::HitsTargetInSky:
    case FC::HitsTargetInSkyGroundsTarget:
    case FC::HoopaRemoveProtectionsBypassSubstituteLowerUserDef1:
    case FC::IgnoreTargetAbility:
    case FC::IgnoreTargetDefSpDefEvaStatStages:
    case FC::LowerPPOfTargetLastMoveBy3:
    case FC::LowerPPOfTargetLastMoveBy4:
    case FC::MultiTurnAttackBideThenReturnDoubleDamage:
    case FC::MultiTurnAttackPreventSleeping:
    case FC::NegateTargetAbility:
    case FC::NegateTargetAbilityIfTargetActed:
    case FC::NormalMovesBecomeElectric:
    case FC::PowerHigherWithConsecutiveUseOnUserSide:
    case FC::PowerHigherWithTargetWeight:
    case FC::PowerHigherWithUserHappiness:
    case FC::PowerHigherWithUserHeavierThanTarget:
    case FC::PowerLowerWithUserHappiness:
    case FC::PowerUpAllyMove:
    case FC::ProtectUserSideFromDamagingMovesIfUserFirstTurn:
    case FC::ProtectUserSideFromMultiTargetDamagingMoves:
    case FC::ProtectUserSideFromPriorityMoves:
    case FC::ProtectUserSideFromStatusMoves:
    case FC::PursueSwitchingFoe:
    case FC::RandomlyDamageOrHealTarget:
    case FC::RedirectAllMovesToTarget:
    case FC::RedirectAllMovesToUser:
    case FC::RemoveProtections:
    case FC::RemoveProtectionsBypassSubstitute:
    case FC::RemoveTargetItem:
    case FC::ReplaceMoveThisBattleWithTargetLastMoveUsed:
    case FC::ReplaceMoveWithTargetLastMoveUsed:
    case FC::RestoreUserConsumedItem:
    case FC::SetAttackerMovePPTo0IfUserFaints:
    case FC::SetTargetAbilityToInsomnia:
    case FC::SetTargetAbilityToSimple:
    case FC::SetTargetAbilityToUserAbility:
    case FC::SetTargetTypesToPsychic:
    case FC::SetTargetTypesToWater:
    case FC::SetUserAbilityToTargetAbility:
    case FC::SetUserTypesBasedOnEnvironment:
    case FC::SetUserTypesToResistLastAttack:
    case FC::SetUserTypesToTargetTypes:
    case FC::SetUserTypesToUserMoveType:
    case FC::StartDamageTargetEachTurnIfTargetAsleep:
    case FC::StartHealUserEachTurnTrapUserInBattle:
    case FC::StartNegateHeldItems:
    case FC::StartNegateTargetEvasionStatStageAndDarkImmunity:
    case FC::StartNegateTargetEvasionStatStageAndGhostImmunity:
    case FC::StartPreventCriticalHitsAgainstUserSide:
    case FC::StartRaiseUserAtk1WhenDamaged:
    case FC::StartSwapAllBattlersBaseDefensiveStats:
    case FC::StartTargetAirborneAndAlwaysHitByMoves:
    case FC::StartTargetCannotUseItem:
    case FC::StartUserAirborne:
    case FC::StartUserSideDoubleSpeed:
    case FC::StartWeakenDamageAgainstUserSideIfHail:
    case FC::StartWeakenElectricMoves:
    case FC::StartWeakenFireMoves:
    case FC::StealAndUseBeneficialStatusMove:
    case FC::SwapSideEffects:
    case FC::TargetActsLast:
    case FC::TargetActsNext:
    case FC::TargetMovesBecomeElectric:
    case FC::TargetNextFireMoveDamagesTarget:
    case FC::TargetTakesUserItem:
    case FC::TargetUsesItsLastUsedMoveAgain:
    case FC::ThrowUserItemAtTarget:
    case FC::TrapAllBattlersInBattleForOneTurn:
    case FC::TrapTargetInBattleLowerTargetDefSpDef1EachTurn:
    case FC::TrapUserAndTargetInBattle:
    case FC::TypeAndPowerDependOnTerrain:
    case FC::TypeAndPowerDependOnUserBerry:
    case FC::TypeAndPowerDependOnWeather:
    case FC::TypeDependsOnUserDrive:
    case FC::TypeDependsOnUserIVs:
    case FC::TypeDependsOnUserMemory:
    case FC::TypeDependsOnUserMorpekoFormRaiseUserSpeed1:
    case FC::TypeDependsOnUserPlate:
    case FC::TypeIsUserFirstType:
    case FC::UseLastMoveUsed:
    case FC::UseLastMoveUsedByTarget:
    case FC::UseMoveDependingOnEnvironment:
    case FC::UseMoveTargetIsAboutToUse:
    case FC::UseRandomMove:
    case FC::UseRandomMoveFromUserParty:
    case FC::UseRandomUserMoveIfAsleep:
    case FC::UseTargetAttackInsteadOfUserAttack:
    case FC::UseTargetDefenseInsteadOfTargetSpDef:
    case FC::UseUserDefenseInsteadOfUserAttack:
    case FC::UsedAfterAllyRoundWithDoublePower:
    case FC::UsedAfterUserTakesPhysicalDamage:
    case FC::UserConsumeBerryRaiseDefense2:
    case FC::UserConsumeTargetBerry:
    case FC::UserFaintsHealAndCureReplacement:
    case FC::UserFaintsHealAndCureReplacementRestorePP:
    case FC::UserLosesFireType:
    case FC::UserSwapsPositionsWithAlly:
    case FC::UserTargetAverageBaseAtkSpAtk:
    case FC::UserTargetAverageBaseDefSpDef:
    case FC::UserTargetSwapAbilities:
    case FC::WaterPledge:
        return true; // unsupported or Gen III-incompatible mechanic; intentional no-op


    case FC::HealUserFullyAndFallAsleep:
        // handled above; keep for exhaustiveness
        return true;
    case FC::AlwaysCriticalHit:
    case FC::DoublePowerAfterFusionBolt:
    case FC::DoublePowerAfterFusionFlare:
    case FC::DoublePowerIfAllyFaintedLastTurn:
    case FC::DoublePowerIfTargetActed:
    case FC::DoublePowerIfTargetHPLessThanHalf:
    case FC::DoublePowerIfTargetInSky:
    case FC::DoublePowerIfTargetLostHPThisTurn:
    case FC::DoublePowerIfTargetNotActed:
    case FC::DoublePowerIfTargetPoisoned:
    case FC::DoublePowerIfTargetStatusProblem:
    case FC::DoublePowerIfTargetUnderground:
    case FC::DoublePowerIfTargetUnderwater:
    case FC::DoublePowerIfUserHasNoItem:
    case FC::DoublePowerIfUserLastMoveFailed:
    case FC::DoublePowerIfUserLostHPThisTurn:
    case FC::DoublePowerIfUserPoisonedBurnedParalyzed:
    case FC::DoublePowerIfUserStatsLoweredThisTurn:
    case FC::DoublePowerInElectricTerrain:
    case FC::PowerDependsOnUserStockpile:
    case FC::PowerHigherWithLessPP:
    case FC::PowerHigherWithTargetFasterThanUser:
    case FC::PowerHigherWithTargetHP:
    case FC::PowerHigherWithTargetPositiveStatStages:
    case FC::PowerHigherWithUserFasterThanTarget:
    case FC::PowerHigherWithUserHP:
    case FC::PowerHigherWithUserPositiveStatStages:
    case FC::PowerLowerWithUserHP:
    case FC::RandomPowerDoublePowerIfTargetUnderground:
        return true; // power handled in moveEffectPower
    case FC::DoublePowerIfTargetAsleepCureTarget:
        if (ctx.target && ctx.target->getStatusCondition() == Pkmn::StatusCondition::SLEEP)
        {
            ctx.target->setStatusCondition(Pkmn::StatusCondition::NONE);
            msgFmt(ctx, boost::format(lc::translate("%1% woke up!")) % ctx.target->getDisplayName());
        }
        return true;
    case FC::DoublePowerIfTargetParalyzedCureTarget:
        if (ctx.target && ctx.target->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
        {
            ctx.target->setStatusCondition(Pkmn::StatusCondition::NONE);
            msgFmt(ctx, boost::format(lc::translate("%1% was cured of paralysis!")) % ctx.target->getDisplayName());
        }
        return true;

    default:
        // Explicit documented no-op / best-effort for remaining Essentials codes
        // (abilities, complex item/type interactions, pledges, etc.).
        return true;
    }
}
