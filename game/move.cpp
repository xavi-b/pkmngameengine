#include "move.h"

#include "game.h"

#include <algorithm>

Move::Move(MoveDef::MoveDefPtr definition) : definition(definition)
{
}

size_t Move::getCurrentPP() const
{
    return currentPP;
}

void Move::setCurrentPP(size_t newCurrentPP)
{
    currentPP = newCurrentPP;
}

void Move::decreasePP()
{
    if (currentPP > 0)
        --currentPP;
}

size_t Move::getPpUps() const
{
    return ppUps;
}

void Move::setPpUps(size_t newPpUps)
{
    ppUps = std::min<size_t>(3, newPpUps);
}

size_t Move::maxPP() const
{
    if (!definition)
        return 0;
    size_t base = definition->getTotalPP();
    return base + base * 20 * ppUps / 100;
}

MoveDef::MoveDefPtr Move::getDefinition() const
{
    return definition;
}

void tag_invoke(js::value_from_tag, js::value& jv, Move::MovePtr const& o)
{
    if (o)
    {
        jv = {
            {"currentPP",  o->currentPP          },
            {"ppUps",      o->ppUps              },
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
    if (!definition)
        return nullptr;
    auto move       = std::make_shared<Move>(definition);
    move->currentPP = js::value_to<size_t>(obj.at("currentPP"));
    if (obj.contains("ppUps"))
        move->ppUps = std::min<size_t>(3, js::value_to<size_t>(obj.at("ppUps")));
    return move;
}
