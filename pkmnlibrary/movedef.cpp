#include "movedef.h"

#include <iostream>

std::string MoveDef::CategoryToString(Category e)
{
    switch (e)
    {
    case PHYSICAL:
        return "PHYSICAL";
    case SPECIAL:
        return "SPECIAL";
    case STATUS:
        return "STATUS";
    case __SIZE:
        return "__SIZE";
    }
    return "???";
}

MoveDef::Category MoveDef::CategoryFromString(std::string e)
{
    std::transform(e.begin(), e.end(), e.begin(), toupper);

    if (e == "SPECIAL")
        return SPECIAL;
    else if (e == "STATUS")
        return STATUS;
    else
        return PHYSICAL;
}

MoveDef::MoveDef()
{
}

MoveDef::MoveDefPtr MoveDef::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto pkmn      = std::make_shared<MoveDef>();
    pkmn->id       = id;
    pkmn->name     = pt.get<std::string>("Name");
    pkmn->type     = pt.get<std::string>("Type");
    pkmn->accuracy = pt.get<size_t>("Accuracy");
    if (pt.count("Power"))
        pkmn->power = pt.get<size_t>("Power");
    pkmn->totalPP  = pt.get<size_t>("TotalPP");
    pkmn->category = CategoryFromString(pt.get<std::string>("Category"));
    if (pt.count("Priority"))
        pkmn->power = pt.get<int>("Priority");

    return pkmn;
}

std::vector<MoveDef::MoveDefPtr> MoveDef::vectorFromPropertyTree(pt::ptree const& pt)
{
    std::vector<MoveDefPtr> pkmns;
    for (auto pos = pt.begin(); pos != pt.end(); ++pos)
    {
        auto p = *pos;
        pkmns.push_back(fromPropertyTree(p.first, p.second));
    }
    return pkmns;
}

std::string MoveDef::getId() const
{
    return id;
}

void MoveDef::setId(std::string const& newId)
{
    id = newId;
}

std::string MoveDef::getName() const
{
    return name;
}

void MoveDef::setName(std::string const& newName)
{
    name = newName;
}

std::string MoveDef::getType() const
{
    return type;
}

void MoveDef::setType(std::string const& newType)
{
    type = newType;
}

size_t MoveDef::getAccuracy() const
{
    return accuracy;
}

void MoveDef::setAccuracy(size_t newAccuracy)
{
    accuracy = newAccuracy;
}

size_t MoveDef::getPower() const
{
    return power;
}

void MoveDef::setPower(size_t newPower)
{
    power = newPower;
}

size_t MoveDef::getTotalPP() const
{
    return totalPP;
}

void MoveDef::setTotalPP(size_t newTotalPP)
{
    totalPP = newTotalPP;
}

MoveDef::Category MoveDef::getCategory() const
{
    return category;
}

void MoveDef::setCategory(Category newCategory)
{
    category = newCategory;
}

int MoveDef::getPriority() const
{
    return priority;
}

void MoveDef::setPriority(int newPriority)
{
    priority = newPriority;
}

void tag_invoke(js::value_from_tag, js::value& jv, MoveDef::MoveDefPtr const& o)
{
    if (o && !o->id.empty())
    {
        jv = {
            {"id",       o->id      },
            {"name",     o->name    },
            {"type",     o->type    },
            {"accuracy", o->accuracy},
            {"power",    o->power   },
            {"totalPP",  o->totalPP },
            {"category", o->category},
            {"priority", o->priority}
        };
    }
    else
    {
        jv = {};
    }
}

MoveDef::MoveDefPtr tag_invoke(js::value_to_tag<MoveDef::MoveDefPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    std::string id = js::value_to<std::string>(obj.at("id"));
    if (id.empty())
        return nullptr;
    else
    {
        auto pkmn      = std::make_shared<MoveDef>();
        pkmn->id       = id;
        pkmn->name     = js::value_to<std::string>(obj.at("name"));
        pkmn->type     = js::value_to<std::string>(obj.at("type"));
        pkmn->accuracy = js::value_to<size_t>(obj.at("accuracy"));
        pkmn->power    = js::value_to<size_t>(obj.at("power"));
        pkmn->totalPP  = js::value_to<size_t>(obj.at("totalPP"));
        pkmn->category = static_cast<MoveDef::Category>(js::value_to<int>(obj.at("category")));
        pkmn->priority = js::value_to<size_t>(obj.at("priority"));
        return pkmn;
    }
}
