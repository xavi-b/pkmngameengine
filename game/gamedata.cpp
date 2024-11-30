#include "gamedata.h"

GameData::GameData()
{
}

Type::TypePtr GameData::typeFor(std::string const& id)
{
    auto it = std::find_if(types.begin(), types.end(), [=](Type::TypePtr const& e) {
        return e->getId() == id;
    });
    if (it != types.end())
        return *it;
    return nullptr;
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

ItemDef::ItemDefPtr GameData::itemDefFor(std::string const& id)
{
    auto it = std::find_if(items.begin(), items.end(), [=](ItemDef::ItemDefPtr const& e) {
        return e->getId() == id;
    });
    if (it != items.end())
        return *it;
    return nullptr;
}
