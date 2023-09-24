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
