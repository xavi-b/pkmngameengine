#include "type.h"

#include <iostream>

Type::Type()
{
}

Type::TypePtr Type::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto type  = std::make_shared<Type>();
    type->id   = id;
    type->name = pt.get<std::string>("Name");

    std::string token;
    char        delimiter = ',';

    std::string        strWeaknesses = pt.get<std::string>("Weaknesses", "");
    std::istringstream tokenStreamWeaknesses(strWeaknesses);
    while (std::getline(tokenStreamWeaknesses, token, delimiter))
        type->weaknesses.push_back(token);
    std::string        strResistances = pt.get<std::string>("Resistances", "");
    std::istringstream tokenStreamResistances(strResistances);
    while (std::getline(tokenStreamResistances, token, delimiter))
        type->resistances.push_back(token);
    std::string        strImmunities = pt.get<std::string>("Immunities", "");
    std::istringstream tokenStream(strImmunities);
    while (std::getline(tokenStream, token, delimiter))
        type->immunities.push_back(token);

    return type;
}

std::vector<Type::TypePtr> Type::vectorFromPropertyTree(pt::ptree const& pt)
{
    std::vector<TypePtr> types;
    for (auto pos = pt.begin(); pos != pt.end(); ++pos)
    {
        auto p = *pos;
        types.push_back(fromPropertyTree(p.first, p.second));
    }
    return types;
}

std::string Type::getId() const
{
    return id;
}

std::string Type::getName() const
{
    return name;
}

std::vector<std::string> Type::getWeaknesses() const
{
    return weaknesses;
}

std::vector<std::string> Type::getResistances() const
{
    return resistances;
}

std::vector<std::string> Type::getImmunities() const
{
    return immunities;
}

void Type::setId(std::string const& newId)
{
    id = newId;
}

void Type::setName(std::string const& newName)
{
    name = newName;
}

void Type::setWeaknesses(std::vector<std::string> const& newWeaknesses)
{
    weaknesses = newWeaknesses;
}

void Type::setResistances(std::vector<std::string> const& newResistances)
{
    resistances = newResistances;
}

void Type::setImmunities(std::vector<std::string> const& newImmunities)
{
    immunities = newImmunities;
}

void tag_invoke(js::value_from_tag, js::value& jv, Type::TypePtr const& o)
{
    if (o && !o->id.empty())
    {
        jv = {
            {"id",          o->id                                                          },
            {"name",        o->name                                                        },
            {"weaknesses",  js::value_from<std::vector<std::string> const&>(o->weaknesses) },
            {"resistances", js::value_from<std::vector<std::string> const&>(o->resistances)},
            {"immunities",  js::value_from<std::vector<std::string> const&>(o->immunities) }
        };
    }
    else
    {
        jv = {};
    }
}

Type::TypePtr tag_invoke(js::value_to_tag<Type::TypePtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    std::string id = js::value_to<std::string>(obj.at("id"));
    if (id.empty())
        return nullptr;
    else
    {
        auto type         = std::make_shared<Type>();
        type->id          = id;
        type->name        = js::value_to<std::string>(obj.at("name"));
        type->weaknesses  = js::value_to<std::vector<std::string>>(obj.at("weaknesses"));
        type->resistances = js::value_to<std::vector<std::string>>(obj.at("resistances"));
        type->immunities  = js::value_to<std::vector<std::string>>(obj.at("immunities"));
        return type;
    }
}
