#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "entities/player.h"
#include "gametime.h"
#include "itemdef.h"
#include "pkmndef.h"
#include "type.h"

#include <string>

class GameData
{
public:
    GameData();

    Type::TypePtr       typeFor(std::string const& id);
    PkmnDef::PkmnDefPtr pkmnDefFor(std::string const& id);
    MoveDef::MoveDefPtr moveDefFor(std::string const& id);
    ItemDef::ItemDefPtr itemDefFor(std::string const& id);

    std::vector<Type::TypePtr>       types;
    std::vector<PkmnDef::PkmnDefPtr> pkmns;
    std::vector<MoveDef::MoveDefPtr> moves;
    std::vector<ItemDef::ItemDefPtr> items;

    Player      player;
    GameTime    time;
    std::string location;
};

#endif // GAMEDATA_H
