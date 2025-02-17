#include "player.h"

Player::Player()
{
}

size_t Player::getPkmnCount() const
{
    return std::count_if(pkmns.begin(), pkmns.end(), [](auto e) {
        return e != nullptr;
    });
}

Player::Gender Player::getGender() const
{
    return gender;
}

void Player::setGender(Gender newGender)
{
    gender = newGender;
}

void tag_invoke(js::value_from_tag, js::value& jv, Player const& o)
{
    jv = {
        {"name", o.name},
        {"gender", static_cast<int>(o.getGender())},
        {"pkmns", js::value_from<std::array<Pkmn::PkmnPtr, 6> const&>(o.pkmns)},
        {"items", js::value_from<std::array<std::vector<Item::ItemPtr>, ItemDef::NumberOfPockets> const&>(o.items)}
    };
}

Player tag_invoke(js::value_to_tag<Player>, js::value const& jv)
{
    Player            p;
    js::object const& obj = jv.as_object();
    p.name                = js::value_to<std::string>(obj.at("name"));
    p.gender              = static_cast<Player::Gender>(js::value_to<int>(obj.at("gender")));
    p.pkmns               = js::value_to<std::array<Pkmn::PkmnPtr, 6>>(obj.at("pkmns"));
    if (obj.contains("items"))
        p.items = js::value_to<std::array<std::vector<Item::ItemPtr>, ItemDef::NumberOfPockets>>(obj.at("items"));
    return p;
}
