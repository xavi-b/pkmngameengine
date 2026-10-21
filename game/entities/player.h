#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "itemdef.h"
#include "pkmn.h"

#include <array>

class Player : public Entity
{
public:
    enum class Gender
    {
        BOY,
        GIRL
    };

    friend void   tag_invoke(js::value_from_tag, js::value& jv, Player const& o);
    friend Player tag_invoke(js::value_to_tag<Player>, js::value const& jv);

    Player();

    size_t getPkmnCount() const;

    Gender getGender() const;
    void   setGender(Gender newGender);

    void addOneItem(ItemDef::ItemDefPtr itemDef);
    bool removeOneItem(ItemDef::ItemDefPtr itemDef);

    std::string name;

    std::array<Pkmn::PkmnPtr, 6> pkmns;

    std::array<std::vector<Item::ItemPtr>, ItemDef::NumberOfPockets> items;

    uint64_t money = 0;
    uint64_t id    = 0;

    // Used for field poison (1 HP every 4 steps). Persist across saves.
    size_t stepCount = 0;

    // Remaining steps of Repel, Super Repel, or Max Repel. Zero means none is active.
    size_t repelSteps = 0;

private:
    Gender gender = Gender::BOY;
};

#endif // PLAYER_H
