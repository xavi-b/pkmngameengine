#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "item.h"
#include "pkmn.h"

#include <array>
#include <memory>
#include <vector>

class Player : public Entity
{
public:
    Player();

    int gender = 0;

    std::string name;

    std::vector<std::unique_ptr<Item>>   items;
    std::array<std::unique_ptr<Pkmn>, 6> pkmns;
};

#endif // PLAYER_H
