#include "confusionstatus.h"

#include "utils.h"

void ConfusionStatus::clear()
{
    active         = false;
    turnsRemaining = 0;
}

bool ConfusionStatus::apply()
{
    if (active)
        return false;
    active         = true;
    turnsRemaining = Utils::randuint(2, 5);
    return true;
}

ConfusionTickResult tickConfusion(ConfusionStatus& status)
{
    if (!status.active)
        return ConfusionTickResult::None;

    // Final turn: snap out and allow the move.
    if (status.turnsRemaining <= 1)
    {
        status.clear();
        return ConfusionTickResult::SnappedOut;
    }

    --status.turnsRemaining;

    // Generations I–VI: 50% chance to hurt itself.
    if (Utils::randuint(0, 1) == 0)
        return ConfusionTickResult::HurtSelf;

    return ConfusionTickResult::Continues;
}

size_t computeConfusionDamage(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn)
        return 0;

    // Same base formula as Gen III damage, but typeless physical power 40,
    // no critical hit, STAB, weather, or type effectiveness.
    float  level = pkmn->getLevel();
    size_t power = 40;
    size_t a     = pkmn->getStats()[PkmnDef::ATTACK];
    size_t d     = pkmn->getStats()[PkmnDef::DEFENSE];
    if (d == 0)
        d = 1;

    // Generations I–IV: burn halves confusion damage.
    float burn =
        pkmn->getStatusCondition() == Pkmn::StatusCondition::BURN ? 0.5f : 1.0f;
    float damage = (((2 * level / 5 + 2) * power * a / d) / 50 + 2) * burn;
    float random = Utils::randuint(85, 100) / 100.0f;
    damage *= random;
    if (damage < 1.0f)
        damage = 1.0f;
    return static_cast<size_t>(damage);
}

bool moveConfusesTarget(MoveDef::MoveDefPtr const& move)
{
    if (!move)
        return false;

    switch (move->getFunctionCode())
    {
    case MoveDef::FunctionCode::ConfuseTarget:
    case MoveDef::FunctionCode::ConfuseTargetAlwaysHitsInRainHitsTargetInSky:
    case MoveDef::FunctionCode::RaiseTargetAttack2ConfuseTarget:
    case MoveDef::FunctionCode::RaiseTargetSpAtk1ConfuseTarget:
        return true;
    default:
        return false;
    }
}

size_t confuseChanceForMove(MoveDef::MoveDefPtr const& move)
{
    if (!moveConfusesTarget(move))
        return 0;

    // Primary confuse effects (status moves) often use EffectChance 0 in PBS data;
    // treat that as always confuse on a successful hit.
    size_t chance = move->getEffectChance();
    return chance == 0 ? 100 : chance;
}

bool isStatusConfuseMove(MoveDef::MoveDefPtr const& move)
{
    return move && move->getCategory() == MoveDef::STATUS && moveConfusesTarget(move);
}
