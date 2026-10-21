#ifndef STATUSCONDITION_H
#define STATUSCONDITION_H

#include "entities/player.h"
#include "map.h"
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

MajorStatusActionResult tickMajorStatus(Pkmn::PkmnPtr const& pkmn, std::vector<std::string>& messages,
                                     class BattleBattler* battler = nullptr);

// Snore and Sleep Talk can be used on a turn that would otherwise be spent asleep.
bool canMoveWhileAsleep(MoveDef::MoveDefPtr const& move);

// Status a move can inflict on the target (NONE if none / special-cased).
Pkmn::StatusCondition statusInflictedByMove(MoveDef::MoveDefPtr const& move);

bool isPrimaryStatusMove(MoveDef::MoveDefPtr const& move);

size_t statusEffectChance(MoveDef::MoveDefPtr const& move);

bool canApplyStatus(Pkmn::PkmnPtr const& target, Pkmn::StatusCondition status, bool corrosion = false,
                    class BattleBattler const* battler = nullptr);

// Applies status if allowed. For BADLY_POISON, toxicCounter is reset to 0.
// Returns false if it fails (already statused, type immune, etc.).
bool applyStatusCondition(Pkmn::PkmnPtr const& target, Pkmn::StatusCondition status, size_t& toxicCounter,
                          bool corrosion = false, class BattleBattler const* battler = nullptr);

// Chance (0-100) roll then apply. Handles Tri Attack random status.
bool tryInflictStatusFromMove(Pkmn::PkmnPtr const& target, MoveDef::MoveDefPtr const& move, size_t& toxicCounter);

// Heatproof halves burn residual damage (1/32).
size_t residualStatusDamage(Pkmn::PkmnPtr const& pkmn, size_t& toxicCounter,
                            class BattleBattler const* battler = nullptr);

// Paralysis halves Speed. Optional battler applies stages, items, and ability Speed.
size_t battleSpeed(Pkmn::PkmnPtr const& pkmn, class BattleBattler const* battler = nullptr,
                  Map::Weather weather = Map::Weather::NONE, bool electricTerrain = false);

bool hasType(Pkmn::PkmnPtr const& pkmn, std::string const& typeId, class BattleBattler const* battler = nullptr);

std::vector<std::string> activeTypes(Pkmn::PkmnPtr const& pkmn, class BattleBattler const* battler = nullptr);

std::string statusConditionName(Pkmn::StatusCondition status);

std::string statusInflictedMessage(Pkmn::PkmnPtr const& target);

void maybeThawFromFireHit(Pkmn::PkmnPtr const& target, MoveDef::MoveDefPtr const& move);

bool tickFieldPoison(Player& player, std::vector<std::string>& messages);

#endif // STATUSCONDITION_H
