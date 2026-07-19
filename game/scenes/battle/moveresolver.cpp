#include "moveresolver.h"

#include "confusionstatus.h"
#include "moveeffects.h"
#include "singlebattlescene.h"
#include "statuscondition.h"
#include "utils.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

namespace
{
bool accuracyCheck(MoveEffectContext const& ctx)
{
    auto def = ctx.def;
    size_t accuracy = def->getAccuracy();
    if (accuracy == 0)
        return true; // never miss

    // Gen III: Acc_stage / Eva_stage
    float acc = ctx.userBattler.accuracyStageMultiplier();
    float eva = ctx.targetBattler.evasionStageMultiplier();
    float chance = accuracy * acc / eva;
    if (ctx.targetBattler.minimized && def->hasFlag(MoveDef::Flag::TramplesMinimize))
        chance = 100;
    if (chance >= 100)
        return true;
    return Utils::randuint(1, 100) <= static_cast<size_t>(chance);
}

bool targetsUser(MoveDef::Target t)
{
    return t == MoveDef::Target::User || t == MoveDef::Target::UserSide || t == MoveDef::Target::UserAndAllies
        || t == MoveDef::Target::UserOrNearAlly || t == MoveDef::Target::AllAllies;
}
} // namespace

MoveResolveResult MoveResolver::resolve(SingleBattleSceneData& data, bool playerIsUser, Move::MovePtr const& move)
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

    // Recharge turn
    if (userBattler.recharging)
    {
        userBattler.recharging = false;
        result.messages.push_back(
            (boost::format(lc::translate("%1% must recharge!")) % user->getDisplayName()).str());
        return result;
    }

    // Flinch
    if (userBattler.flinched)
    {
        userBattler.flinched = false;
        result.messages.push_back(
            (boost::format(lc::translate("%1% flinched and couldn't move!")) % user->getDisplayName()).str());
        return result;
    }

    // Protect on target
    MoveEffectContext ctx(data, playerIsUser, user, target, userBattler, targetBattler, move, def);

    // Two-turn charging first turn: apply effect without damaging
    bool finishingTwoTurn = isTwoTurnChargingMove(def->getFunctionCode()) && userBattler.charging;
    if (isTwoTurnChargingMove(def->getFunctionCode()) && !userBattler.charging)
    {
        // Solar Beam etc. skip charge in sun
        bool skipCharge = def->getFunctionCode() == MoveDef::FunctionCode::TwoTurnAttackOneTurnInSun
                       && data.weather == Map::HARSH_SUNLIGHT;
        if (!skipCharge)
        {
            move->decreasePP();
            applyMoveFunctionCode(ctx);
            result.messages = ctx.messages;
            return result;
        }
    }

    if (!finishingTwoTurn)
        move->decreasePP();

    if (finishingTwoTurn)
    {
        userBattler.charging         = false;
        userBattler.semiInvulnerable = false;
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

    if (targetBattler.protect && needsAccuracy && def->hasFlag(MoveDef::Flag::CanProtect))
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% protected itself!")) % target->getDisplayName()).str());
        if (def->getFunctionCode() == MoveDef::FunctionCode::CrashDamageIfFailsUnusableInGravity)
            user->decreaseHP(std::max<size_t>(1, user->getTotalHP() / 2));
        return result;
    }

    if (needsAccuracy && target && !accuracyCheck(ctx))
    {
        result.messages.push_back(lc::translate("The attack missed!"));
        if (def->getFunctionCode() == MoveDef::FunctionCode::CrashDamageIfFailsUnusableInGravity)
        {
            user->decreaseHP(std::max<size_t>(1, user->getTotalHP() / 2));
            result.messages.push_back(
                (boost::format(lc::translate("%1% kept going and crashed!")) % user->getDisplayName()).str());
            result.userFainted = user->isKO();
        }
        return result;
    }

    result.hit = true;
    ctx.moveHit = true;

    // Pure status / self moves without damage
    if (def->getCategory() == MoveDef::STATUS || def->getPower() == 0)
    {
        applyMoveFunctionCode(ctx);
        result.messages.insert(result.messages.end(), ctx.messages.begin(), ctx.messages.end());
        result.shouldSwitchUser   = ctx.shouldSwitchUser;
        result.shouldSwitchTarget = ctx.shouldSwitchTarget;
        result.forcedSwitchTarget = ctx.forcedSwitchTarget;
        result.userFainted        = user->isKO();
        result.targetFainted      = target && target->isKO();
        return result;
    }

    if (!target)
        return result;

    int hits = moveHitCount(def);
    size_t totalDamage = 0;
    for (int hit = 0; hit < hits; ++hit)
    {
        size_t basePower = def->getPower();
        size_t power     = moveEffectPower(ctx, basePower);
        size_t damage    = SingleBattleScene::computeDamage(user, target, move, data.weather, &userBattler,
                                                            &targetBattler, power, false);
        if (def->getFunctionCode() == MoveDef::FunctionCode::AlwaysCriticalHit
            || def->getFunctionCode() == MoveDef::FunctionCode::HitThreeTimesAlwaysCriticalHit)
        {
            damage = SingleBattleScene::computeDamage(user, target, move, data.weather, &userBattler, &targetBattler,
                                                      power, false);
        }

        if (targetBattler.substituteHP > 0)
        {
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
            target->decreaseHP(damage);
            totalDamage += damage;
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

    if (hits > 1 && !target->isKO())
        result.messages.push_back((boost::format(lc::translate("Hit %1% time(s)!")) % hits).str());

    ctx.damageDealt = totalDamage;
    applyMoveFunctionCode(ctx);
    result.messages.insert(result.messages.end(), ctx.messages.begin(), ctx.messages.end());
    result.shouldSwitchUser   = ctx.shouldSwitchUser;
    result.shouldSwitchTarget = ctx.shouldSwitchTarget;
    result.forcedSwitchTarget = ctx.forcedSwitchTarget;
    result.targetFainted      = target->isKO();
    result.userFainted        = user->isKO();

    // Destiny Bond
    if (result.userFainted && userBattler.destinyBond && target && !target->isKO())
    {
        target->setHP(0);
        result.targetFainted = true;
        result.messages.push_back(lc::translate("It took its attacker down with it!"));
    }

    return result;
}
