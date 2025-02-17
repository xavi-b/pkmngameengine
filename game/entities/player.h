#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "item.h"
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

    std::string name;

    std::array<Pkmn::PkmnPtr, 6> pkmns;

    std::array<std::vector<Item::ItemPtr>, ItemDef::NumberOfPockets> items;

private:
    Gender gender = Gender::BOY;
};

#endif // PLAYER_H
