#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <string>
#include "entities/player.h"
#include "gametime.h"

class GameData
{
public:
    GameData();

    Player   player;
    GameTime time;
};

#endif // GAMEDATA_H
