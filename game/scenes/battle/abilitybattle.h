#ifndef ABILITYBATTLE_H
#define ABILITYBATTLE_H

#include "battlebattler.h"
#include "map.h"
#include "movedef.h"
#include "pkmn.h"
#include "singlebattlescenedata.h"

#include <string>
#include <vector>

// Copies the species ability onto the battler when it is sent out.
void assignBattleAbility(Pkmn::PkmnPtr const& pkmn, BattleBattler& battler);

// Multitype and RKS System follow the held plate or memory. An empty item leaves the species type.
void refreshPlateType(Pkmn::PkmnPtr const& pkmn, BattleBattler& battler);

// Forecast, Flower Gift, Hunger Switch, Schooling, Zen Mode, and Shields Down.
// Zen Mode only updates when allowZen is true (end of turn).
void refreshForme(SingleBattleSceneData& data, bool playerSide, bool allowZen = false);

// Toggles Hunger Switch, then refreshes both formes (including Zen Mode).
void tickFormes(SingleBattleSceneData& data);

enum class AbilityMoment
{
    SwitchIn,
    BeforeMove,
    OnHit,
    OnStatus,
    Residual
};

struct AbilityEvent
{
    bool                  playerSide = true;
    bool                  contact    = false;
    Pkmn::StatusCondition status     = Pkmn::StatusCondition::NONE;
    std::string           moveType;
    std::string           moveId;
    bool                  blocked      = false;
    bool                  ignoreAbility = false;
    bool                  userFainted  = false;
    std::vector<std::string> messages;
};

// One entry point for send-in, the moment before a move, a hit, a status attempt, and the residual.
void triggerAbility(SingleBattleSceneData& data, AbilityMoment moment, AbilityEvent& event);

// Switch-in abilities such as Intimidate. Messages are appended.
void abilityOnSwitchIn(SingleBattleSceneData& data, bool playerSide, std::vector<std::string>& messages);

// Before the move. Sets event.blocked when Truant loafs.
void abilityBeforeMove(SingleBattleSceneData& data, bool playerIsUser, std::vector<std::string>& messages, bool& blocked);

void abilityOnResidual(SingleBattleSceneData& data, bool playerSide, std::vector<std::string>& messages);

// Contact and on-hit abilities. Returns true if the attacker fainted.
bool abilityOnHit(SingleBattleSceneData& data,
                  bool                    attackerIsPlayer,
                  bool                    contact,
                  std::string const&      moveType,
                  std::string const&      moveId,
                  std::vector<std::string>& messages);

// Immunity, Limber, and the other status-blocking abilities.
// fromOther=false for self-inflicted orbs (Flower Veil does not block those).
bool abilityBlocksStatus(Pkmn::PkmnPtr const& target, BattleBattler const& battler, Pkmn::StatusCondition status,
                         bool fromOther = true);

// Overgrow, Huge Power, Swift Swim, and the other passive modifiers.
float abilityPowerMultiplier(Pkmn::PkmnPtr const& user, BattleBattler const& battler, MoveDef::MoveDefPtr const& move,
                             Map::Weather weather);
float abilityTypePowerMultiplier(Pkmn::PkmnPtr const& user, BattleBattler const& battler, std::string const& type);
float abilitySpeedMultiplier(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler, Map::Weather weather);

// Levitate, Flash Fire, and the type-absorbing abilities. Heals are applied here.
void abilityOnSwitchOut(SingleBattleSceneData& data, bool playerSide);

// Frees the foe from Mean Look / binds / Jaw Lock / ability traps when this side leaves.
void clearTrapsOnLeave(SingleBattleSceneData& data, bool playerSide);

float abilityOutgoingMultiplier(Pkmn::PkmnPtr const& user, BattleBattler const& battler, MoveDef::MoveDefPtr const& move,
                                bool targetMoved, Map::Weather weather, float effectiveness = 1.f, bool critical = false,
                                std::string moveType = {});
float abilityIncomingMultiplier(Pkmn::PkmnPtr const& target, BattleBattler const& battler, std::string const& moveType,
                                float effectiveness, bool critical, MoveDef::Category category, bool contact,
                                bool ignoreAbility = false, bool strongWinds = false);

// Mold Breaker, Teravolt, Turboblaze, and moves that ignore the target's ability.
// Mold Breaker is off while that ability is suppressed.
bool moveIgnoresTargetAbility(BattleBattler const& user, MoveDef::FunctionCode code);

void applyMimicry(SingleBattleSceneData& data, std::vector<std::string>& messages);

void abilityOnTargetFaint(SingleBattleSceneData& data, bool attackerIsPlayer, std::vector<std::string>& messages);

bool abilityGrantsImmunity(SingleBattleSceneData& data,
                           Pkmn::PkmnPtr const&    target,
                           BattleBattler&         targetBattler,
                           std::string const&     moveType,
                           bool                   ignoreAbility,
                           std::vector<std::string>& messages,
                           bool                   pierceGroundImmunity = false);

// Primordial Sea / Desolate Land / Delta Stream block ordinary weather changes.
bool primordialWeatherBlocksNewWeather(SingleBattleSceneData const& data);

// Name shown in battle text (Illusion / Transform disguise when present).
std::string battleDisplayName(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler);

// Predicted Illusion name before switch-in ability runs (last non-KO party mate).
std::string predictedIllusionName(Pkmn::PkmnPtr const& pkmn, bool playerSide, SingleBattleSceneData const& data);

#endif // ABILITYBATTLE_H
