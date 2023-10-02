#include "gamedata.h"

GameData::GameData()
{
}

PkmnDef::PkmnDefPtr GameData::pkmnDefFor(std::string const& id)
{
    auto it = std::find_if(pkmns.begin(), pkmns.end(), [=](PkmnDef::PkmnDefPtr const& e) {
        return e->getId() == id;
    });
    if (it != pkmns.end())
        return *it;
    return nullptr;
}

MoveDef::MoveDefPtr GameData::moveDefFor(std::string const& id)
{
    auto it = std::find_if(moves.begin(), moves.end(), [=](MoveDef::MoveDefPtr const& e) {
        return e->getId() == id;
    });
    if (it != moves.end())
        return *it;
    return nullptr;
}
