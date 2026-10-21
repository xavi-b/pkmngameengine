#ifndef PKMNUTILS_H
#define PKMNUTILS_H

#include "entities/player.h"
#include "pkmn.h"

#include <SDL_pixels.h>

namespace PkmnUtils
{
std::string canEvolve(Pkmn::PkmnPtr const& pkmn);
// Matching EvolutionType::Item, using the item id stored on the evolution.
std::string canEvolveByItem(Pkmn::PkmnPtr const& pkmn, std::string const& itemId);

// Places the empty-slot companion (Nincada's Shedinja) when an evolution succeeds.
void grantEmptySlotEvolution(Pkmn::PkmnPtr const& pkmn);

SDL_Color genderColor(Pkmn::Gender gender);
SDL_Color genderColor(Player::Gender gender);
SDL_Color genderBackgroundColor(Player::Gender gender);
wchar_t   genderSymbol(Pkmn::Gender gender);
} // namespace PkmnUtils

#endif // PKMNUTILS_H
