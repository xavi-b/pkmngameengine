#ifndef ITEMUTILS_H
#define ITEMUTILS_H

#include "entities/player.h"
#include "item.h"
#include "pkmn.h"

#include <string>

struct BattleBattler;
struct ConfusionStatus;

namespace ItemUtils
{
struct HeldItemTickResult
{
    bool        used     = false; // berry consumed / effect applied
    bool        fainted  = false;
    std::string message;
};

bool useItemOnPkmn(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target, bool inBattle);
bool useItemOnMove(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target); // Ether / Leppa / Max Ether
bool useItemOnBattler(Item::ItemPtr const& item,
                      Pkmn::PkmnPtr const&  target,
                      BattleBattler&       battler,
                      ConfusionStatus*     confusion = nullptr);
// Guard Spec., escape items, Poké Flute. endsBattle set for dolls/toys.
bool useBattleItemDirect(Item::ItemPtr const& item,
                         Pkmn::PkmnPtr const&  playerPkmn,
                         BattleBattler&       playerBattler,
                         bool&                endsBattle,
                         std::string&         message);

bool useCaptureBall(Item::ItemPtr const& ball, Pkmn::PkmnPtr const& foePkmn);
bool useItemInField(Player& player, Item::ItemPtr const& item);
bool allowFaintedTarget(Item::ItemPtr const& item);
bool curesConfusion(std::string const& itemId);
bool isEscapeItem(std::string const& itemId);

// End-of-turn / pinch held-item effects (Leftovers, Oran, status berries, …).
HeldItemTickResult tickHeldItem(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler, ConfusionStatus* confusion);

void addPkmnToPlayer(Pkmn::PkmnPtr const& pkmn);
} // namespace ItemUtils

#endif // ITEMUTILS_H
