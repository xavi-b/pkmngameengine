#ifndef CONFUSIONSTATUS_H
#define CONFUSIONSTATUS_H

#include "movedef.h"
#include "pkmn.h"

#include <cstddef>

// https://bulbapedia.bulbagarden.net/wiki/Confusion_(status_condition)
struct ConfusionStatus
{
    bool   active         = false;
    size_t turnsRemaining = 0;

    void clear();
    // Returns false if already confused (application fails).
    bool apply();
};

enum class ConfusionTickResult
{
    None,
    SnappedOut,
    HurtSelf,
    Continues
};

// Tick at the start of a move attempt. Gen III: 50% chance to hurt itself.
ConfusionTickResult tickConfusion(ConfusionStatus& status);

// Typeless physical power-40 self-hit; no crit/STAB/type; Gen III burn halves.
size_t computeConfusionDamage(Pkmn::PkmnPtr const& pkmn);

bool moveConfusesTarget(MoveDef::MoveDefPtr const& move);

// Chance (0-100) that the move confuses its target on a successful hit. 0 = never.
size_t confuseChanceForMove(MoveDef::MoveDefPtr const& move);

bool isStatusConfuseMove(MoveDef::MoveDefPtr const& move);

#endif // CONFUSIONSTATUS_H
