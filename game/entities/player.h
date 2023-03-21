#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <memory>
#include <array>
#include "entity.h"
#include "item.h"
#include "pkmn.h"

class Player : public Entity
{
public:
    Player();

    int gender = 0;

    std::string name;

    std::vector<std::unique_ptr<Item>>   items;
    std::array<std::unique_ptr<Pkmn>, 4> pkmns;
};

#endif // PLAYER_H
