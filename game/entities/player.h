#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "pkmn.h"

#include <array>

class Player : public Entity
{
public:
    enum Gender
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

private:
    Gender gender = BOY;
};

#endif // PLAYER_H
