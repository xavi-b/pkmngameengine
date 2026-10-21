#include "pkmnutils.h"

#include "game.h"
#include "utils.h"

#include <iostream>
#include <vector>

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

namespace
{
bool levelMatches(Pkmn::PkmnPtr const& pkmn, std::string const& data)
{
    if (!pkmn || data.empty())
        return false;
    return pkmn->getLevel() == static_cast<size_t>(std::stoi(data));
}
} // namespace

std::string canEvolve(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn || !pkmn->getDefinition())
        return {};

    auto stats = pkmn->getStats();

    std::vector<std::string> possibleEvolutions;

    auto evos = pkmn->getDefinition()->getEvolutions();
    for (auto const& e : evos)
    {
        switch (e.first)
        {
        case PkmnDef::AttackGreater:
            if (levelMatches(pkmn, e.second.data))
            {
                if (stats[PkmnDef::ATTACK] > stats[PkmnDef::DEFENSE])
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::DefenseGreater:
            if (levelMatches(pkmn, e.second.data))
            {
                if (stats[PkmnDef::ATTACK] < stats[PkmnDef::DEFENSE])
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::AtkDefEqual:
            if (levelMatches(pkmn, e.second.data))
            {
                if (stats[PkmnDef::ATTACK] == stats[PkmnDef::DEFENSE])
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::Happiness:
            if (levelMatches(pkmn, e.second.data))
            {
                if (pkmn->getHappiness() >= Pkmn::HighFriendship)
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::HasInParty:
            if (levelMatches(pkmn, e.second.data))
            {
                bool hasInParty = false;
                for (auto const& partyMember : Game::instance()->data.player.pkmns)
                {
                    if (partyMember && partyMember->getDefinition()->getId() == e.second.data)
                    {
                        hasInParty = true;
                        break;
                    }
                }
                if (hasInParty)
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::HasMove:
            if (levelMatches(pkmn, e.second.data))
            {
                bool hasMove = false;
                for (auto const& move : pkmn->getMoves())
                {
                    if (move && move->getDefinition() && move->getDefinition()->getId() == e.second.data)
                    {
                        hasMove = true;
                        break;
                    }
                }
                if (hasMove)
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::Level:
            if (levelMatches(pkmn, e.second.data))
                possibleEvolutions.push_back(e.second.pkmnId);
            break;
        case PkmnDef::LevelMale:
            if (levelMatches(pkmn, e.second.data))
            {
                if (pkmn->isMale())
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::LevelFemale:
            if (levelMatches(pkmn, e.second.data))
            {
                if (pkmn->isFemale())
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::HasEmptySlotInParty:
            break;
        default:
            break;
        }
    }

    if (possibleEvolutions.empty())
        return {};

    size_t index = Utils::randuint(0, possibleEvolutions.size() - 1);
    return possibleEvolutions[index];
}

std::string canEvolveByItem(Pkmn::PkmnPtr const& pkmn, std::string const& itemId)
{
    if (!pkmn || !pkmn->getDefinition() || itemId.empty())
        return {};

    for (auto const& e : pkmn->getDefinition()->getEvolutions())
    {
        if (e.first == PkmnDef::Item && e.second.data == itemId)
            return e.second.pkmnId;
    }
    return {};
}

void grantEmptySlotEvolution(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn || !pkmn->getDefinition())
        return;

    auto& party = Game::instance()->data.player.pkmns;
    for (auto const& e : pkmn->getDefinition()->getEvolutions())
    {
        if (e.first != PkmnDef::HasEmptySlotInParty)
            continue;
        if (!levelMatches(pkmn, e.second.data))
            continue;

        size_t slot = 0;
        for (; slot < party.size(); ++slot)
        {
            if (!party[slot])
                break;
        }
        if (slot >= party.size())
            continue;

        PkmnDef::PkmnDefPtr newPkmnDef = Game::instance()->data.pkmnDefFor(e.second.pkmnId);
        if (!newPkmnDef)
            continue;

        Pkmn::PkmnPtr companion = std::make_shared<Pkmn>(newPkmnDef, pkmn->getLevel());
        companion->generateFromPkmnDef();
        party[slot] = companion;
    }
}

} // namespace PkmnUtils
