#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "entities/player.h"
#include "gametime.h"

#include <string>

class GameData
{
public:
    GameData();

    Player   player;
    GameTime time;
};

#endif // GAMEDATA_H
