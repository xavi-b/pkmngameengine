#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "entities/player.h"
#include "gametime.h"
#include "pkmndef.h"
#include "type.h"

#include <string>

class GameData
{
public:
    GameData();

    PkmnDef::PkmnDefPtr pkmnDefFor(std::string const& id);
    MoveDef::MoveDefPtr moveDefFor(std::string const& id);

    std::vector<Type::TypePtr>       types;
    std::vector<PkmnDef::PkmnDefPtr> pkmns;
    std::vector<MoveDef::MoveDefPtr> moves;

    Player   player;
    GameTime time;
};

#endif // GAMEDATA_H
