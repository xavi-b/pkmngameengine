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
    case __SIZE_STAT:
        return "__SIZE_STAT";
    }
    return "???";
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
    case __SIZE_GROWTH_RATE:
        return "__SIZE_GROWTH_RATE";
    }
    return "???";
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

std::string PkmnDef::EvolutionTypeToString(EvolutionType e)
{
    switch (e)
    {
    case AttackGreater:
        return "AttackGreater";
    case DefenseGreater:
        return "DefenseGreater";
    case AtkDefEqual:
        return "AtkDefEqual";
    case DayHoldItem:
        return "DayHoldItem";
    case Happiness:
        return "Happiness";
    case HappinessDay:
        return "HappinessDay";
    case HappinessNight:
        return "HappinessNight";
    case HasInParty:
        return "HasInParty";
    case HasMove:
        return "HasMove";
    case Item:
        return "Item";
    case Level:
        return "Level";
    case LevelMale:
        return "LevelMale";
    case LevelFemale:
        return "LevelFemale";
    case NightHoldItem:
        return "NightHoldItem";
    case HasEmptySlotInParty:
        return "HasEmptySlotInParty";
    case Trade:
        return "Trade";
    case TradeItem:
        return "TradeItem";
    case TradeSpecies:
        return "TradeSpecies";
    case __SIZE_EVOLUTIONS:
        return "__SIZE_EVOLUTIONS";
    }
    return "???";
}

PkmnDef::EvolutionType PkmnDef::EvolutionTypeFromString(std::string const& e)
{
    if (e == "AttackGreater")
        return AttackGreater;
    if (e == "DefenseGreater")
        return DefenseGreater;
    if (e == "AtkDefEqual")
        return AtkDefEqual;
    if (e == "DayHoldItem")
        return DayHoldItem;
    if (e == "Happiness")
        return Happiness;
    if (e == "HappinessDay")
        return HappinessDay;
    if (e == "HappinessNight")
        return HappinessNight;
    if (e == "HasInParty")
        return HasInParty;
    if (e == "HasMove")
        return HasMove;
    if (e == "Item")
        return Item;
    if (e == "Level")
        return Level;
    if (e == "LevelMale")
        return LevelMale;
    if (e == "LevelFemale")
        return LevelFemale;
    if (e == "NightHoldItem")
        return NightHoldItem;
    if (e == "HasEmptySlotInParty")
        return HasEmptySlotInParty;
    if (e == "Trade")
        return Trade;
    if (e == "TradeItem")
        return TradeItem;
    if (e == "TradeSpecies")
        return TradeSpecies;
    else
        return __SIZE_EVOLUTIONS;
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
    pkmn->baseExp    = pt.get<size_t>("BaseExp");
    pkmn->catchRate  = pt.get<size_t>("CatchRate");
    pkmn->happiness  = pt.get<size_t>("Happiness");

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
        size_t level = std::stoi(token);
        if (std::getline(tokenStreamMovesToLearn, token, delimiter))
            pkmn->movesToLearn.push_back({level, token});
    }

    std::string        strBaseStats = pt.get<std::string>("BaseStats", "");
    std::istringstream tokenStreamBaseStats(strBaseStats);
    size_t             i = 0;
    while (std::getline(tokenStreamBaseStats, token, delimiter))
    {
        pkmn->baseStats[static_cast<Stat>(i)] = std::stoi(token);
        ++i;
    }

    std::string        strEVsToLearn = pt.get<std::string>("EVs", "");
    std::istringstream tokenStreamEVsToLearn(strEVsToLearn);
    while (std::getline(tokenStreamEVsToLearn, token, delimiter))
    {
        Stat stat = StatFromString(token);
        if (std::getline(tokenStreamEVsToLearn, token, delimiter))
            pkmn->EVsToLearn[stat] = std::stoi(token);
    }

    if (pt.count("Evolutions"))
    {
        std::string        strEvolutions = pt.get<std::string>("Evolutions", "");
        std::istringstream tokenStreamEvolutions(strEvolutions);
        while (std::getline(tokenStreamEvolutions, token, delimiter))
        {
            EvolutionType type = __SIZE_EVOLUTIONS;
            Evolution     evolution;
            evolution.pkmnId = token;
            if (std::getline(tokenStreamEvolutions, token, delimiter))
                type = EvolutionTypeFromString(token);
            if (std::getline(tokenStreamEvolutions, token, delimiter))
                evolution.data = token;
            if (type != __SIZE_EVOLUTIONS)
                pkmn->evolutions.emplace(type, evolution);
        }
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

std::vector<PkmnDef::MoveToLearn> const& PkmnDef::getMovesToLearn() const
{
    return movesToLearn;
}

void PkmnDef::setMovesToLearn(std::vector<MoveToLearn> const& newMovesToLearn)
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

PkmnDef::GrowthRate PkmnDef::getGrowthRate() const
{
    return growthRate;
}

void PkmnDef::setGrowthRate(GrowthRate newGrowthRate)
{
    growthRate = newGrowthRate;
}

size_t PkmnDef::getBaseExp() const
{
    return baseExp;
}

void PkmnDef::setBaseExp(size_t newBaseExp)
{
    baseExp = newBaseExp;
}

size_t PkmnDef::getCatchRate() const
{
    return catchRate;
}

void PkmnDef::setCatchRate(size_t newCatchRate)
{
    catchRate = newCatchRate;
}

unsigned char PkmnDef::getHappiness() const
{
    return happiness;
}

void PkmnDef::setHappiness(unsigned char newHappiness)
{
    happiness = newHappiness;
}

std::multimap<PkmnDef::EvolutionType, PkmnDef::Evolution> const& PkmnDef::getEvolutions() const
{
    return evolutions;
}

void PkmnDef::setEvolutions(std::multimap<EvolutionType, Evolution> const& newEvolutions)
{
    evolutions = newEvolutions;
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
                {"level", e.level},
                {"move",  e.id   }
            });
        js::array jsBaseStats;
        for (auto const& e : o->baseStats)
            jsBaseStats.push_back(js::value{
                {"id",    e.first },
                {"value", e.second}
            });
        js::array jsEvolutions;
        for (auto const& e : o->evolutions)
            jsEvolutions.push_back(js::value{
                {"type",   e.first        },
                {"pkmnId", e.second.pkmnId},
                {"data",   e.second.data  }
            });
        jv = {
            {"id",           o->id         },
            {"name",         o->name       },
            {"growthRate",   o->growthRate },
            {"types",        jsTypes       },
            {"movesToLearn", jsMovesToLearn},
            {"stats",        jsBaseStats   },
            {"baseExp",      o->baseExp    },
            {"catchRate",    o->catchRate  },
            {"happiness",    o->happiness  },
            {"evolutions",   jsEvolutions  }
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
        pkmn->growthRate = static_cast<PkmnDef::GrowthRate>(js::value_to<size_t>(obj.at("growthRate")));
        for (auto& value : obj.at("types").as_array())
            pkmn->types.push_back(js::value_to<std::string>(value));
        for (auto& value : obj.at("movesToLearn").as_array())
        {
            js::object const& obj   = value.as_object();
            size_t            level = js::value_to<size_t>(obj.at("level"));
            std::string       id    = js::value_to<std::string>(obj.at("move"));
            pkmn->movesToLearn.push_back({level, id});
        }
        for (auto& value : obj.at("stats").as_array())
        {
            js::object const& obj = value.as_object();
            pkmn->baseStats[static_cast<PkmnDef::Stat>(js::value_to<size_t>(obj.at("id")))] =
                js::value_to<size_t>(obj.at("value"));
        }
        pkmn->baseExp   = js::value_to<size_t>(obj.at("baseExp"));
        pkmn->catchRate = js::value_to<size_t>(obj.at("catchRate"));
        pkmn->happiness = js::value_to<unsigned char>(obj.at("happiness"));
        for (auto& value : obj.at("evolutions").as_array())
        {
            js::object const& obj = value.as_object();

            PkmnDef::EvolutionType type = static_cast<PkmnDef::EvolutionType>(js::value_to<size_t>(obj.at("type")));
            PkmnDef::Evolution     evolution;
            evolution.pkmnId = js::value_to<std::string>(obj.at("pkmnId"));
            evolution.data   = js::value_to<std::string>(obj.at("data"));
            pkmn->evolutions.emplace(type, evolution);
        }
        return pkmn;
    }
}
