#ifndef ITEMUTILS_H
#define ITEMUTILS_H

#include "entities/player.h"
#include "item.h"
#include "map.h"
#include "movedef.h"
#include "pkmn.h"

#include <string>
#include <vector>

struct BattleBattler;
struct ConfusionStatus;
struct SingleBattleSceneData;

namespace ItemUtils
{
struct HeldItemTickResult
{
    bool        used     = false; // berry consumed / effect applied
    bool        fainted  = false;
    bool        ejected  = false;
    std::string message;
};

bool useItemOnPkmn(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target, bool inBattle);
bool useItemOnMove(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target); // Ether / Leppa / Max Ether
// Ether, Max Ether, PP Up, and PP Max need the player to choose a move first.
bool needsFieldMoveChoice(std::string const& itemId);
bool useMoveItem(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target, int slot);
bool useItemOnBattler(Item::ItemPtr const& item,
                      Pkmn::PkmnPtr const&  target,
                      BattleBattler&       battler,
                      ConfusionStatus*     confusion,
                      SingleBattleSceneData& data,
                      std::vector<std::string>& messages);
// Guard Spec., escape items, Poké Flute. endsBattle set for dolls/toys.
bool useBattleItemDirect(Item::ItemPtr const& item,
                         Pkmn::PkmnPtr const&  playerPkmn,
                         BattleBattler&       playerBattler,
                         bool&                endsBattle,
                         std::string&         message,
                         int&                 sideMistTurns);

struct CaptureContext
{
    int          turn            = 1;
    bool         atNight         = false;
    bool         inCave          = false;
    bool         underwater      = false;
    bool         fishing         = false;
    size_t       playerLevel     = 0;
    std::string  playerSpeciesId;
    Pkmn::Gender playerGender = Pkmn::Gender::GENDERLESS;
};

bool useCaptureBall(Item::ItemPtr const& ball, Pkmn::PkmnPtr const& foePkmn, CaptureContext const& context);
bool useItemInField(Player& player, Item::ItemPtr const& item, std::string& message);
bool allowFaintedTarget(Item::ItemPtr const& item);
bool curesConfusion(std::string const& itemId);
bool isEscapeItem(std::string const& itemId);

// End-of-turn / pinch held-item effects (Leftovers, Oran, status berries, …).
HeldItemTickResult tickHeldItem(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler, ConfusionStatus* confusion,
                             SingleBattleSceneData& data);

// Clears negative stages and consumes White Herb when one is held.
HeldItemTickResult consumeWhiteHerb(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler);

// Eats a pinch berry when HP is low enough. Does nothing for other items.
// ignorePinch feeds the held berry immediately, for Bug Bite and Pluck.
HeldItemTickResult consumePinchBerry(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler, ConfusionStatus* confusion,
                                      SingleBattleSceneData& data, bool ignorePinch = false);

// Lum / Pecha / etc. immediately after a status is inflicted.
HeldItemTickResult consumeStatusBerry(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler, ConfusionStatus* confusion);

// Gen 5+: Mental Herb cures Attract / Taunt / Encore / Disable / Torment / Heal Block as they land.
void consumeMentalHerb(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler, std::vector<std::string>& messages);

// Bookkeeping when a held item is lost mid-battle (Unburden / Harvest).
void noteHeldItemLoss(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler, std::string const& itemId);

// The eater consumes `berry` without replacing their own held item.
HeldItemTickResult eatBerry(Pkmn::PkmnPtr const& eater, BattleBattler* battler, ConfusionStatus* confusion,
                            Item::ItemPtr const& berry, SingleBattleSceneData& data);

void addPkmnToPlayer(Pkmn::PkmnPtr const& pkmn);

// Life Orb, Muscle Band, and Wise Glasses.
float heldPowerMultiplier(Pkmn::PkmnPtr const& pkmn, MoveDef::MoveDefPtr const& move,
                          BattleBattler const* battler = nullptr);

// Charcoal, plates, drives, memories, and the legendary orbs. `type` is the move's final type.
float heldTypePowerMultiplier(Pkmn::PkmnPtr const& pkmn, std::string const& type,
                              BattleBattler const* battler = nullptr);

// Choice Scarf.
float heldSpeedMultiplier(Pkmn::PkmnPtr const& pkmn, BattleBattler const* battler = nullptr);

// Wide Lens and Zoom Lens.
float heldAccuracyMultiplier(Pkmn::PkmnPtr const& pkmn, bool targetAlreadyMoved, BattleBattler const* battler = nullptr);

// Plate, drive, and memory item ids mapped to the move type they represent.
std::string plateTypeForItem(std::string const& itemId);

// Light Clay stretches screens from 5 turns to 8.
int screenTurnsForHolder(Pkmn::PkmnPtr const& user, BattleBattler const* battler = nullptr);

// Air Balloon stops a Ground move, then pops.
bool airBalloonBlocksGround(Pkmn::PkmnPtr const& target,
                            std::string const&   moveType,
                            std::vector<std::string>& messages,
                            BattleBattler const* battler = nullptr);

// Weakness Policy, Eject Button, Red Card, and a popped balloon.
void applyDamagingHitItems(Pkmn::PkmnPtr const&      target,
                           BattleBattler&           battler,
                           float                    effectiveness,
                           std::string const&       moveType,
                           bool&                    switchHolder,
                           bool&                    forceAttackerOut,
                           std::vector<std::string>& messages,
                           SingleBattleSceneData&   data);

// Returns the item id that should extend this weather, or empty.
int weatherTurnsForHolder(Pkmn::PkmnPtr const& user, Map::Weather weather, BattleBattler const* battler = nullptr);

// Focus Sash and resist berries may shrink the hit. Messages are appended.
size_t reduceIncomingDamage(Pkmn::PkmnPtr const& target,
                            std::string const&   moveType,
                            float                effectiveness,
                            size_t               damage,
                            std::vector<std::string>& messages,
                            BattleBattler* battler = nullptr);

// Life Orb / Shell Bell (once per move) and Rocky Helmet / Sticky Barb (per contact hit).
// skipPostMoveItems: only contact retaliation (for multi-hit loops).
// Returns true if the attacker fainted.
bool applyContactItemEffects(Pkmn::PkmnPtr const& attacker,
                             Pkmn::PkmnPtr const& defender,
                             bool                 contact,
                             bool                 attackerDealtDamage,
                             size_t               damageDealt,
                             std::vector<std::string>& messages,
                             BattleBattler const* attackerBattler = nullptr,
                             BattleBattler const* defenderBattler = nullptr,
                             bool                 suppressLifeOrb = false,
                             bool                 skipPostMoveItems = false);

// Empty while Magic Room, Embargo, or Klutz suppresses the held item.
std::string activeHeldId(Pkmn::PkmnPtr const& pkmn, BattleBattler const* battler = nullptr);
} // namespace ItemUtils

#endif // ITEMUTILS_H
