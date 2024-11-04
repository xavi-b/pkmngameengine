#include "pkmnutils.h"

#include <iostream>

namespace PkmnUtils
{

SDL_Color genderColor(Pkmn::Gender gender)
{
    switch (gender)
    {
    case Pkmn::Gender::MALE:
        return {78, 208, 251, 255};
    case Pkmn::Gender::FEMALE:
        return {247, 152, 144, 255};
    default:
        return {};
    }
}

wchar_t genderSymbol(Pkmn::Gender gender)
{
    switch (gender)
    {
    case Pkmn::Gender::MALE:
        return L'♂';
    case Pkmn::Gender::FEMALE:
        return L'♀';
    default:
        return L' ';
    }
}

} // namespace PkmnUtils
