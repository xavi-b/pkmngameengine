#include "pkmndef.h"

#include <iostream>

std::string PkmnDef::StatToString(Stat e)
{
    switch (e)
    {
    case HP:
        return "HP";
    case ATTACK:
        return "ATTACK";
    case DEFENSE:
        return "DEFENSE";
    case SPECIAL_ATTACK:
        return "SPECIAL_ATTACK";
    case SPECIAL_DEFENSE:
        return "SPECIAL_DEFENSE";
    case SPEED:
        return "SPEED";
    default:
        return "???";
    }
}

PkmnDef::Stat PkmnDef::StatFromString(std::string const& e)
{
    if (e == "ATTACK")
        return ATTACK;
    else if (e == "DEFENSE")
        return DEFENSE;
    else if (e == "SPECIAL_ATTACK")
        return SPECIAL_ATTACK;
    else if (e == "SPECIAL_ATTACK")
        return SPECIAL_DEFENSE;
    else if (e == "SPEED")
        return SPEED;
    else
        return HP;
}

std::string PkmnDef::GrowthRateToString(GrowthRate e)
{
    switch (e)
    {
    case ERRATIC:
        return "ERRATIC";
    case FAST:
        return "FAST";
    case MEDIUM_FAST:
        return "MEDIUM_FAST";
    case MEDIUM_SLOW:
        return "MEDIUM_SLOW";
    case SLOW:
        return "SLOW";
    case FLUCTUATING:
        return "FLUCTUATING";
    default:
        return "???";
    }
}

PkmnDef::GrowthRate PkmnDef::GrowthRateFromString(std::string const& e)
{
    if (e == "Erratic" || e == "ERRATIC")
        return ERRATIC;
    else if (e == "Fast" || e == "FAST")
        return FAST;
    else if (e == "Parabolic" || e == "MEDIUM_FAST")
        return MEDIUM_FAST;
    else if (e == "Medium" || e == "MEDIUM_SLOW")
        return MEDIUM_SLOW;
    else if (e == "Slow" || e == "SLOW")
        return SLOW;
    else
        return FLUCTUATING;
}

PkmnDef::PkmnDef()
{
}

PkmnDef::PkmnDefPtr PkmnDef::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto pkmn        = std::make_shared<PkmnDef>();
    pkmn->id         = id;
    pkmn->name       = pt.get<std::string>("Name");
    pkmn->growthRate = GrowthRateFromString(pt.get<std::string>("GrowthRate"));

    std::string token;
    char        delimiter = ',';

    std::string        strTypes = pt.get<std::string>("Types", "");
    std::istringstream tokenStreamTypes(strTypes);
    while (std::getline(tokenStreamTypes, token, delimiter))
        pkmn->types.push_back(token);

    std::string        strMovesToLearn = pt.get<std::string>("Moves", "");
    std::istringstream tokenStreamMovesToLearn(strMovesToLearn);
    while (std::getline(tokenStreamMovesToLearn, token, delimiter))
    {
        int level = std::stoi(token);
        if (std::getline(tokenStreamMovesToLearn, token, delimiter))
            pkmn->movesToLearn[level] = token;
    }

    std::string        strBaseStats = pt.get<std::string>("BaseStats", "");
    std::istringstream tokenStreamBaseStats(strBaseStats);
    int                i = 0;
    while (std::getline(tokenStreamBaseStats, token, delimiter))
    {
        pkmn->baseStats[static_cast<Stat>(i)] = std::stoi(token);
        ++i;
    }

    std::string        strEVsToLearn = pt.get<std::string>("Moves", "");
    std::istringstream tokenStreamEVsToLearn(strEVsToLearn);
    while (std::getline(tokenStreamEVsToLearn, token, delimiter))
    {
        Stat stat = StatFromString(token);
        if (std::getline(tokenStreamEVsToLearn, token, delimiter))
            pkmn->EVsToLearn[stat] = std::stoi(token);
    }

    return pkmn;
}

std::vector<PkmnDef::PkmnDefPtr> PkmnDef::vectorFromPropertyTree(pt::ptree const& pt)
{
    std::vector<PkmnDefPtr> pkmns;
    for (auto pos = pt.begin(); pos != pt.end(); ++pos)
    {
        auto p = *pos;
        pkmns.push_back(fromPropertyTree(p.first, p.second));
    }
    return pkmns;
}

std::string PkmnDef::getId() const
{
    return id;
}

void PkmnDef::setId(std::string const& newId)
{
    id = newId;
}

std::string PkmnDef::getName() const
{
    return name;
}

void PkmnDef::setName(std::string const& newName)
{
    name = newName;
}

std::vector<std::string> const& PkmnDef::getTypes() const
{
    return types;
}

void PkmnDef::setTypes(std::vector<std::string> const& newTypes)
{
    types = newTypes;
}

std::map<int, std::string> const& PkmnDef::getMovesToLearn() const
{
    return movesToLearn;
}

void PkmnDef::setMovesToLearn(std::map<int, std::string> const& newMovesToLearn)
{
    movesToLearn = newMovesToLearn;
}

std::map<PkmnDef::Stat, size_t> const& PkmnDef::getBaseStats() const
{
    return baseStats;
}

void PkmnDef::setBaseStats(std::map<PkmnDef::Stat, size_t> const& newBaseStats)
{
    baseStats = newBaseStats;
}

std::map<PkmnDef::Stat, size_t> const& PkmnDef::getEVsToLearn() const
{
    return EVsToLearn;
}

void PkmnDef::setEVsToLearn(std::map<PkmnDef::Stat, size_t> const& newEVsToLearn)
{
    EVsToLearn = newEVsToLearn;
}

void tag_invoke(js::value_from_tag, js::value& jv, PkmnDef::PkmnDefPtr const& o)
{
    if (o && !o->id.empty())
    {
        js::array jsTypes;
        for (auto const& e : o->types)
            jsTypes.push_back(js::value(e));
        js::array jsMovesToLearn;
        for (auto const& e : o->movesToLearn)
            jsMovesToLearn.push_back(js::value{
                {"level", e.first },
                {"move",  e.second}
            });
        js::array jsBaseStats;
        for (auto const& e : o->baseStats)
            jsBaseStats.push_back(js::value{
                {"id",    e.first },
                {"value", e.second}
            });
        jv = {
            {"id",           o->id         },
            {"name",         o->name       },
            {"growthRate",   o->growthRate },
            {"types",        jsTypes       },
            {"movesToLearn", jsMovesToLearn},
            {"stats",        jsBaseStats   }
        };
    }
    else
    {
        jv = {};
    }
}

PkmnDef::PkmnDefPtr tag_invoke(js::value_to_tag<PkmnDef::PkmnDefPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    std::string id = js::value_to<std::string>(obj.at("id"));
    if (id.empty())
        return nullptr;
    else
    {
        auto pkmn        = std::make_shared<PkmnDef>();
        pkmn->id         = id;
        pkmn->name       = js::value_to<std::string>(obj.at("name"));
        pkmn->growthRate = static_cast<PkmnDef::GrowthRate>(js::value_to<int>(obj.at("growthRate")));
        for (auto& value : obj.at("types").as_array())
            pkmn->types.push_back(js::value_to<std::string>(value));
        for (auto& value : obj.at("moves").as_array())
        {
            js::object const& obj                                     = value.as_object();
            pkmn->movesToLearn[js::value_to<size_t>(obj.at("level"))] = js::value_to<std::string>(obj.at("move"));
        }
        for (auto& value : obj.at("stats").as_array())
        {
            js::object const& obj = value.as_object();
            pkmn->baseStats[static_cast<PkmnDef::Stat>(js::value_to<size_t>(obj.at("id")))] =
                js::value_to<size_t>(obj.at("value"));
        }
        return pkmn;
    }
}
