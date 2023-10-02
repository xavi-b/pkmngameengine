#include "move.h"

#include "game.h"

Move::Move(MoveDef::MoveDefPtr definition) : definition(definition)
{
}

void tag_invoke(js::value_from_tag, js::value& jv, Move::MovePtr const& o)
{
    if (o)
    {
        jv = {
            {"currentPP",  o->currentPP          },
            {"definition", o->definition->getId()}
        };
    }
    else
    {
        jv = {};
    }
}

Move::MovePtr tag_invoke(js::value_to_tag<Move::MovePtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    auto definition = Game::instance()->data.moveDefFor(js::value_to<std::string>(obj.at("definition")));
    auto move       = std::make_shared<Move>(definition);
    move->currentPP = js::value_to<size_t>(obj.at("currentPP"));
    return move;
}
