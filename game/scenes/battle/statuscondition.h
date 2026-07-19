#ifndef STATUSCONDITION_H
#define STATUSCONDITION_H

#include "entities/player.h"
#include "movedef.h"
#include "pkmn.h"

#include <cstddef>
#include <string>
#include <vector>

// https://bulbapedia.bulbagarden.net/wiki/Status_condition
// Mechanics follow Generation III (matching this engine's damage formula).

enum class MajorStatusActionResult
{
    CanAct,
    Prevented,
    WokeUp,
    Thawed
};

// Sleep / freeze / paralysis check before a Pokémon tries to move.
MajorStatusActionResult tickMajorStatus(Pkmn::PkmnPtr const& pkmn, std::vector<std::string>& messages);

// Status a move can inflict on the target (NONE if none / special-cased).
Pkmn::StatusCondition statusInflictedByMove(MoveDef::MoveDefPtr const& move);

bool isPrimaryStatusMove(MoveDef::MoveDefPtr const& move);

size_t statusEffectChance(MoveDef::MoveDefPtr const& move);

bool canApplyStatus(Pkmn::PkmnPtr const& target, Pkmn::StatusCondition status);

// Applies status if allowed. For BADLY_POISON, toxicCounter is reset to 0.
// Returns false if it fails (already statused, type immune, etc.).
bool applyStatusCondition(Pkmn::PkmnPtr const& target, Pkmn::StatusCondition status, size_t& toxicCounter);

// Chance (0-100) roll then apply. Handles Tri Attack random status.
bool tryInflictStatusFromMove(Pkmn::PkmnPtr const& target, MoveDef::MoveDefPtr const& move, size_t& toxicCounter);

size_t residualStatusDamage(Pkmn::PkmnPtr const& pkmn, size_t& toxicCounter);

// Gen I–VI: paralysis cuts Speed to 25%. Optional battler applies Speed stages.
size_t battleSpeed(Pkmn::PkmnPtr const& pkmn, class BattleBattler const* battler = nullptr);

bool hasType(Pkmn::PkmnPtr const& pkmn, std::string const& typeId);

std::string statusConditionName(Pkmn::StatusCondition status);

std::string statusInflictedMessage(Pkmn::PkmnPtr const& target);

// Fire-type damaging hits thaw a frozen target (Gen III+).
void maybeThawFromFireHit(Pkmn::PkmnPtr const& target, MoveDef::MoveDefPtr const& move);

// Gen I–IV / Essentials: poisoned party Pokémon lose 1 HP every 4 steps outside battle.
// Appends faint messages. Returns true if any Pokémon took damage.
bool tickFieldPoison(Player& player, std::vector<std::string>& messages);

#endif // STATUSCONDITION_H
