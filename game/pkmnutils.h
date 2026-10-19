#ifndef PKMNUTILS_H
#define PKMNUTILS_H

#include "entities/player.h"
#include "pkmn.h"

#include <SDL_pixels.h>

namespace PkmnUtils
{
// Level-up evolution the Pokémon can perform right now. Empty when none applies.
// HasEmptySlotInParty is not returned here; it is a companion created only after
// the evolution actually commits.
std::string canEvolve(Pkmn::PkmnPtr const& pkmn);

// Places the empty-slot companion (Nincada's Shedinja) when an evolution succeeds.
void grantEmptySlotEvolution(Pkmn::PkmnPtr const& pkmn);

SDL_Color genderColor(Pkmn::Gender gender);
SDL_Color genderColor(Player::Gender gender);
SDL_Color genderBackgroundColor(Player::Gender gender);
wchar_t   genderSymbol(Pkmn::Gender gender);
} // namespace PkmnUtils

#endif // PKMNUTILS_H
