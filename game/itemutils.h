#ifndef ITEMUTILS_H
#define ITEMUTILS_H

#include "entities/player.h"
#include "item.h"
#include "pkmn.h"

namespace ItemUtils
{
bool consumePlayerItem(Player& player, Item::ItemPtr const& item);
bool useItemOnPkmn(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target, bool inBattle);
bool useCaptureBall(Item::ItemPtr const& ball, Pkmn::PkmnPtr const& foePkmn);
bool useItemInField(Player& player, Item::ItemPtr const& item);
bool allowFaintedTarget(Item::ItemPtr const& item);
void addPkmnToPlayer(Pkmn::PkmnPtr const& pkmn);
} // namespace ItemUtils

#endif // ITEMUTILS_H
