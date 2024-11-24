#ifndef PKMNUTILS_H
#define PKMNUTILS_H

#include "entities/player.h"
#include "pkmn.h"

#include <SDL_pixels.h>

namespace PkmnUtils
{
SDL_Color genderColor(Pkmn::Gender gender);
SDL_Color genderColor(Player::Gender gender);
SDL_Color genderBackgroundColor(Player::Gender gender);
wchar_t   genderSymbol(Pkmn::Gender gender);
} // namespace PkmnUtils

#endif // PKMNUTILS_H
