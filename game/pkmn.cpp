#include "pkmn.h"

#include "game.h"

Pkmn::Pkmn(PkmnDef::PkmnDefPtr definition, size_t level) : definition(definition), level(level)
{
}

void tag_invoke(js::value_from_tag, js::value& jv, Pkmn::PkmnPtr const& o)
{
    if (o)
    {
        jv = {
            {"moves", js::value_from<std::array<Move::MovePtr, 4> const&>(o->moves)},
            {"level", o->level},
            {"hp", o->hp},
            {"nickName", o->nickName},
            {"definition", o->definition->getId()}
        };
    }
    else
    {
        jv = {};
    }
}

Pkmn::PkmnPtr tag_invoke(js::value_to_tag<Pkmn::PkmnPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    auto   definition = Game::instance()->data.pkmnDefFor(js::value_to<std::string>(obj.at("definition")));
    size_t level      = js::value_to<size_t>(obj.at("level"));
    auto   pkmn       = std::make_shared<Pkmn>(definition, level);
    pkmn->hp          = js::value_to<size_t>(obj.at("hp"));
    pkmn->nickName    = js::value_to<std::string>(obj.at("nickName"));
    pkmn->moves       = js::value_to<std::array<Move::MovePtr, 4>>(obj.at("moves"));
    return pkmn;
}
