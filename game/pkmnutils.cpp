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

SDL_Color genderColor(Player::Gender gender)
{
    switch (gender)
    {
    case Player::Gender::BOY:
        return {59, 144, 219, 255};
    case Player::Gender::GIRL:
        return {255, 73, 92, 255};
    default:
        return {};
    }
}

SDL_Color genderBackgroundColor(Player::Gender gender)
{
    switch (gender)
    {
    case Player::Gender::BOY:
        return {56, 128, 202, 255};
    case Player::Gender::GIRL:
        return {240, 40, 61, 255};
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
