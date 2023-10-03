#include "pkmn.h"

#include "game.h"

Pkmn::Pkmn(PkmnDef::PkmnDefPtr definition, size_t level) : definition(definition), level(level)
{
}

std::map<PkmnDef::Stat, size_t> Pkmn::getStats()
{
    std::map<PkmnDef::Stat, size_t> baseStats = definition->getBaseStats();
    std::map<PkmnDef::Stat, size_t> stats;
    stats[PkmnDef::HP] =
        (((2 * baseStats[PkmnDef::HP] + IVs[PkmnDef::HP] + EVs[PkmnDef::HP] / 4) * level) / 100) + level + 10;
    for (size_t i = static_cast<int>(PkmnDef::HP) + 1; i < PkmnDef::StatCount; ++i)
    {
        PkmnDef::Stat stat = static_cast<PkmnDef::Stat>(i);
        stats[PkmnDef::HP] = ((((2 * baseStats[stat] + IVs[stat] + EVs[stat] / 4) * level) / 100) + 5); // TODO * nature
    }
    return stats;
}

std::map<PkmnDef::Stat, size_t> const& Pkmn::getIVs() const
{
    return IVs;
}

void Pkmn::setIVs(std::map<PkmnDef::Stat, size_t> const& newIVs)
{
    IVs = newIVs;
}

std::map<PkmnDef::Stat, size_t> const& Pkmn::getEVs() const
{
    return EVs;
}

void Pkmn::setEVs(std::map<PkmnDef::Stat, size_t> const& newEVs)
{
    EVs = newEVs;
}

void tag_invoke(js::value_from_tag, js::value& jv, Pkmn::PkmnPtr const& o)
{
    if (o)
    {
        js::array jsIVs;
        for (auto const& e : o->IVs)
            jsIVs.push_back(js::value{
                {"id",    e.first },
                {"value", e.second}
            });
        js::array jsEVs;
        for (auto const& e : o->EVs)
            jsEVs.push_back(js::value{
                {"id",    e.first },
                {"value", e.second}
            });
        jv = {
            {"moves", js::value_from<std::array<Move::MovePtr, 4> const&>(o->moves)},
            {"level", o->level},
            {"exp", o->exp},
            {"hp", o->hp},
            {"nickName", o->nickName},
            {"definition", o->definition->getId()},
            {"IVs", jsIVs},
            {"EVs", jsEVs},
            {"statusCondition", o->statusCondition}
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

    auto   definition     = Game::instance()->data.pkmnDefFor(js::value_to<std::string>(obj.at("definition")));
    size_t level          = js::value_to<size_t>(obj.at("level"));
    auto   pkmn           = std::make_shared<Pkmn>(definition, level);
    pkmn->exp             = js::value_to<size_t>(obj.at("exp"));
    pkmn->hp              = js::value_to<size_t>(obj.at("hp"));
    pkmn->nickName        = js::value_to<std::string>(obj.at("nickName"));
    pkmn->moves           = js::value_to<std::array<Move::MovePtr, 4>>(obj.at("moves"));
    pkmn->statusCondition = static_cast<Pkmn::StatusCondition>(js::value_to<int>(obj.at("statusCondition")));
    for (auto& value : obj.at("IVs").as_array())
    {
        js::object const& obj                                                  = value.as_object();
        pkmn->IVs[static_cast<PkmnDef::Stat>(js::value_to<int>(obj.at("id")))] = js::value_to<size_t>(obj.at("value"));
    }
    for (auto& value : obj.at("EVs").as_array())
    {
        js::object const& obj                                                  = value.as_object();
        pkmn->EVs[static_cast<PkmnDef::Stat>(js::value_to<int>(obj.at("id")))] = js::value_to<size_t>(obj.at("value"));
    }
    return pkmn;
}
