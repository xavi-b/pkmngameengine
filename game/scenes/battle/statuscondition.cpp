#include "statuscondition.h"

#include "abilitybattle.h"
#include "battlebattler.h"
#include "itemutils.h"
#include "utils.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

std::vector<std::string> activeTypes(Pkmn::PkmnPtr const& pkmn, BattleBattler const* battler)
{
    if (battler && !battler->typesOverride.empty())
        return battler->typesOverride;
    if (pkmn && pkmn->getDefinition())
        return pkmn->getDefinition()->getTypes();
    return {};
}

bool hasType(Pkmn::PkmnPtr const& pkmn, std::string const& typeId, BattleBattler const* battler)
{
    auto types = activeTypes(pkmn, battler);
    return std::find(types.begin(), types.end(), typeId) != types.end();
}

std::string statusConditionName(Pkmn::StatusCondition status)
{
    switch (status)
    {
    case Pkmn::StatusCondition::BURN:
        return "burned";
    case Pkmn::StatusCondition::FREEZE:
        return "frozen solid";
    case Pkmn::StatusCondition::PARALYSIS:
        return "paralyzed";
    case Pkmn::StatusCondition::POISON:
        return "poisoned";
    case Pkmn::StatusCondition::BADLY_POISON:
        return "badly poisoned";
    case Pkmn::StatusCondition::SLEEP:
        return "asleep";
    case Pkmn::StatusCondition::NONE:
    default:
        return "";
    }
}

std::string statusInflictedMessage(Pkmn::PkmnPtr const& target)
{
    if (!target)
        return {};
    auto status = target->getStatusCondition();
    if (status == Pkmn::StatusCondition::SLEEP)
    {
        boost::format text = boost::format(lc::translate("%1% fell asleep!")) % target->getDisplayName();
        return text.str();
    }
    boost::format text =
        boost::format(lc::translate("%1% was %2%!")) % target->getDisplayName() % statusConditionName(status);
    return text.str();
}

bool canMoveWhileAsleep(MoveDef::MoveDefPtr const& move)
{
    if (!move)
        return false;
    auto code = move->getFunctionCode();
    return code == MoveDef::FunctionCode::FlinchTargetFailsIfUserNotAsleep
        || code == MoveDef::FunctionCode::UseRandomUserMoveIfAsleep;
}

MajorStatusActionResult tickMajorStatus(Pkmn::PkmnPtr const& pkmn, std::vector<std::string>& messages,
                                     BattleBattler* battler)
{
    if (!pkmn)
        return MajorStatusActionResult::CanAct;

    auto status = pkmn->getStatusCondition();

    if (status == Pkmn::StatusCondition::SLEEP)
    {
        size_t step = 1;
        if (battler && !battler->abilitySuppressed && battler->abilityId == "EARLYBIRD")
            step = 2;
        if (pkmn->getStatusTurns() <= step)
        {
            pkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
            if (battler)
                battler->nightmare = false;
            boost::format text = boost::format(lc::translate("%1% woke up!")) % pkmn->getDisplayName();
            messages.push_back(text.str());
            return MajorStatusActionResult::WokeUp;
        }

        pkmn->setStatusTurns(pkmn->getStatusTurns() - step);
        boost::format text = boost::format(lc::translate("%1% is fast asleep.")) % pkmn->getDisplayName();
        messages.push_back(text.str());
        return MajorStatusActionResult::Prevented;
    }

    if (status == Pkmn::StatusCondition::FREEZE)
    {
        // Gen III+: 20% chance to thaw each turn.
        if (Utils::randuint(1, 100) <= 20)
        {
            pkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
            boost::format text = boost::format(lc::translate("%1% thawed out!")) % pkmn->getDisplayName();
            messages.push_back(text.str());
            return MajorStatusActionResult::Thawed;
        }

        boost::format text = boost::format(lc::translate("%1% is frozen solid!")) % pkmn->getDisplayName();
        messages.push_back(text.str());
        return MajorStatusActionResult::Prevented;
    }

    if (status == Pkmn::StatusCondition::PARALYSIS)
    {
        // 25% chance to be fully paralyzed.
        if (Utils::randuint(1, 100) <= 25)
        {
            boost::format text =
                boost::format(lc::translate("%1% is paralyzed! It can't move!")) % pkmn->getDisplayName();
            messages.push_back(text.str());
            return MajorStatusActionResult::Prevented;
        }
    }

    return MajorStatusActionResult::CanAct;
}

Pkmn::StatusCondition statusInflictedByMove(MoveDef::MoveDefPtr const& move)
{
    if (!move)
        return Pkmn::StatusCondition::NONE;

    switch (move->getFunctionCode())
    {
    case MoveDef::FunctionCode::BurnTarget:
    case MoveDef::FunctionCode::BurnFlinchTarget:
    case MoveDef::FunctionCode::BurnTargetIfTargetStatsRaisedThisTurn:
    case MoveDef::FunctionCode::RecoilThirdOfDamageDealtBurnTarget:
    case MoveDef::FunctionCode::TwoTurnAttackBurnTarget:
        return Pkmn::StatusCondition::BURN;

    case MoveDef::FunctionCode::FreezeTarget:
    case MoveDef::FunctionCode::FreezeTargetAlwaysHitsInHail:
    case MoveDef::FunctionCode::FreezeTargetSuperEffectiveAgainstWater:
    case MoveDef::FunctionCode::FreezeFlinchTarget:
        return Pkmn::StatusCondition::FREEZE;

    case MoveDef::FunctionCode::ParalyzeTarget:
    case MoveDef::FunctionCode::ParalyzeTargetAlwaysHitsInRainHitsTargetInSky:
    case MoveDef::FunctionCode::ParalyzeTargetIfNotTypeImmune:
    case MoveDef::FunctionCode::ParalyzeFlinchTarget:
    case MoveDef::FunctionCode::RecoilThirdOfDamageDealtParalyzeTarget:
    case MoveDef::FunctionCode::TwoTurnAttackParalyzeTarget:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyParalyzeTarget:
        return Pkmn::StatusCondition::PARALYSIS;

    case MoveDef::FunctionCode::PoisonTarget:
    case MoveDef::FunctionCode::PoisonTargetLowerTargetSpeed1:
    case MoveDef::FunctionCode::HitTwoTimesPoisonTarget:
    case MoveDef::FunctionCode::CategoryDependsOnHigherDamagePoisonTarget:
        return Pkmn::StatusCondition::POISON;

    case MoveDef::FunctionCode::BadPoisonTarget:
        return Pkmn::StatusCondition::BADLY_POISON;

    case MoveDef::FunctionCode::SleepTarget:
    case MoveDef::FunctionCode::SleepTargetIfUserDarkrai:
    case MoveDef::FunctionCode::SleepTargetChangeUserMeloettaForm:
    case MoveDef::FunctionCode::HealUserFullyAndFallAsleep:
        return Pkmn::StatusCondition::SLEEP;

    default:
        return Pkmn::StatusCondition::NONE;
    }
}

bool isPrimaryStatusMove(MoveDef::MoveDefPtr const& move)
{
    if (!move || move->getCategory() != MoveDef::STATUS)
        return false;
    auto code = move->getFunctionCode();
    if (code == MoveDef::FunctionCode::ParalyzeBurnOrFreezeTarget)
        return false;
    if (code == MoveDef::FunctionCode::HealUserFullyAndFallAsleep)
        return false; // Rest targets the user
    return statusInflictedByMove(move) != Pkmn::StatusCondition::NONE;
}

size_t statusEffectChance(MoveDef::MoveDefPtr const& move)
{
    if (!move)
        return 0;
    size_t chance = move->getEffectChance();
    return chance == 0 ? 100 : chance;
}

bool canApplyStatus(Pkmn::PkmnPtr const& target, Pkmn::StatusCondition status, bool corrosion,
                    BattleBattler const* battler)
{
    if (!target || status == Pkmn::StatusCondition::NONE)
        return false;
    if (target->getStatusCondition() != Pkmn::StatusCondition::NONE)
        return false;

    switch (status)
    {
    case Pkmn::StatusCondition::BURN:
        return !hasType(target, "FIRE", battler);
    case Pkmn::StatusCondition::FREEZE:
        return !hasType(target, "ICE", battler);
    case Pkmn::StatusCondition::POISON:
    case Pkmn::StatusCondition::BADLY_POISON:
        if (corrosion)
            return true;
        return !hasType(target, "POISON", battler) && !hasType(target, "STEEL", battler);
    case Pkmn::StatusCondition::PARALYSIS:
        return true;
    case Pkmn::StatusCondition::SLEEP:
        return true;
    default:
        return false;
    }
}

bool applyStatusCondition(Pkmn::PkmnPtr const& target, Pkmn::StatusCondition status, size_t& toxicCounter, bool corrosion,
                          BattleBattler const* battler)
{
    if (!canApplyStatus(target, status, corrosion, battler))
        return false;

    target->setStatusCondition(status);
    if (status == Pkmn::StatusCondition::SLEEP)
        target->setStatusTurns(Utils::randuint(2, 4));
    else
        target->setStatusTurns(0);

    if (status == Pkmn::StatusCondition::BADLY_POISON)
        toxicCounter = 0;

    return true;
}

bool tryInflictStatusFromMove(Pkmn::PkmnPtr const& target, MoveDef::MoveDefPtr const& move, size_t& toxicCounter)
{
    if (!target || !move || target->isKO())
        return false;

    Pkmn::StatusCondition status = statusInflictedByMove(move);

    if (move->getFunctionCode() == MoveDef::FunctionCode::ParalyzeBurnOrFreezeTarget)
    {
        if (Utils::randuint(1, 100) > statusEffectChance(move))
            return false;
        switch (Utils::randuint(0, 2))
        {
        case 0:
            status = Pkmn::StatusCondition::PARALYSIS;
            break;
        case 1:
            status = Pkmn::StatusCondition::BURN;
            break;
        default:
            status = Pkmn::StatusCondition::FREEZE;
            break;
        }
    }
    else
    {
        if (status == Pkmn::StatusCondition::NONE)
            return false;
        if (Utils::randuint(1, 100) > statusEffectChance(move))
            return false;
    }

    return applyStatusCondition(target, status, toxicCounter);
}

size_t residualStatusDamage(Pkmn::PkmnPtr const& pkmn, size_t& toxicCounter, BattleBattler const* battler)
{
    if (!pkmn || pkmn->isKO())
        return 0;

    size_t maxHp = std::max<size_t>(1, pkmn->getTotalHP());

    switch (pkmn->getStatusCondition())
    {
    case Pkmn::StatusCondition::BURN:
        (void)battler;
        return std::max<size_t>(1, maxHp / 8);
    case Pkmn::StatusCondition::POISON:
        // Gen II+: 1/8 max HP
        return std::max<size_t>(1, maxHp / 8);
    case Pkmn::StatusCondition::BADLY_POISON:
        // N/16 max HP, N increases each turn (starts at 1).
        if (toxicCounter < 15)
            ++toxicCounter;
        return std::max<size_t>(1, (maxHp * toxicCounter) / 16);
    default:
        return 0;
    }
}

size_t battleSpeed(Pkmn::PkmnPtr const& pkmn, BattleBattler const* battler, Map::Weather weather, bool electricTerrain)
{
    if (!pkmn)
        return 1;
    size_t speed = pkmn->getStats().at(PkmnDef::SPEED);
    if (battler && battler->speedOverride >= 0)
        speed = static_cast<size_t>(battler->speedOverride);
    if (battler)
        speed = battler->effectiveStat(speed, BattleBattler::StageStat::Speed);
    (void)electricTerrain;
    if (pkmn->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
        speed = speed / 4;
    speed = static_cast<size_t>(static_cast<float>(speed) * ItemUtils::heldSpeedMultiplier(pkmn, battler));
    if (battler)
        speed = static_cast<size_t>(static_cast<float>(speed) * abilitySpeedMultiplier(pkmn, *battler, weather));
    return std::max<size_t>(1, speed);
}

void maybeThawFromFireHit(Pkmn::PkmnPtr const& target, MoveDef::MoveDefPtr const& move)
{
    if (!target || !move || target->getStatusCondition() != Pkmn::StatusCondition::FREEZE)
        return;
    if (move->getCategory() == MoveDef::STATUS)
        return;
    if (move->getType() == "FIRE")
        target->setStatusCondition(Pkmn::StatusCondition::NONE);
}

bool tickFieldPoison(Player& player, std::vector<std::string>& messages)
{
    bool damaged = false;

    for (auto& pkmn : player.pkmns)
    {
        if (!pkmn || pkmn->isKO())
            continue;

        auto status = pkmn->getStatusCondition();
        if (status != Pkmn::StatusCondition::POISON && status != Pkmn::StatusCondition::BADLY_POISON)
            continue;

        pkmn->decreaseHP(1);
        damaged = true;

        if (pkmn->isKO())
        {
            pkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
            boost::format text = boost::format(lc::translate("%1% fainted...")) % pkmn->getDisplayName();
            messages.push_back(text.str());
        }
    }

    return damaged;
}
