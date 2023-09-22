#include "type.h"

#include <iostream>

Type::Type()
{
}

Type::TypePtr Type::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto type  = std::make_unique<Type>();
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

void tag_invoke(js::value_from_tag, js::value& jv, Type::TypePtr const& o)
{
    if (o && !o->id.empty())
    {
        js::array jsWeaknesses;
        for (auto const& e : o->weaknesses)
            jsWeaknesses.push_back(js::value(e));
        js::array jsResistances;
        for (auto const& e : o->resistances)
            jsResistances.push_back(js::value(e));
        js::array jsImmunities;
        for (auto const& e : o->immunities)
            jsImmunities.push_back(js::value(e));
        jv = {
            {"id",          o->id        },
            {"name",        o->name      },
            {"weaknesses",  jsWeaknesses },
            {"resistances", jsResistances},
            {"immunities",  jsImmunities }
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
        auto type  = std::make_unique<Type>();
        type->id   = id;
        type->name = js::value_to<std::string>(obj.at("name"));
        for (auto& value : obj.at("weaknesses").as_array())
            type->weaknesses.push_back(js::value_to<std::string>(value));
        for (auto& value : obj.at("resistances").as_array())
            type->resistances.push_back(js::value_to<std::string>(value));
        for (auto& value : obj.at("immunities").as_array())
            type->immunities.push_back(js::value_to<std::string>(value));
        return type;
    }
}
