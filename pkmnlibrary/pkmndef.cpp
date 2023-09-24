#include "pkmndef.h"

#include <iostream>

PkmnDef::PkmnDef()
{
}

PkmnDef::PkmnDefPtr PkmnDef::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto pkmn  = std::make_shared<PkmnDef>();
    pkmn->id   = id;
    pkmn->name = pt.get<std::string>("Name");

    std::string token;
    char        delimiter = ',';

    std::string        strTypes = pt.get<std::string>("Types", "");
    std::istringstream tokenStreamTypes(strTypes);
    while (std::getline(tokenStreamTypes, token, delimiter))
        pkmn->types.push_back(token);

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

std::string PkmnDef::getName() const
{
    return name;
}

std::vector<std::string> PkmnDef::getTypes() const
{
    return types;
}

void PkmnDef::setId(std::string const& newId)
{
    id = newId;
}

void PkmnDef::setName(std::string const& newName)
{
    name = newName;
}

void PkmnDef::setTypes(std::vector<std::string> const& newTypes)
{
    types = newTypes;
}

void tag_invoke(js::value_from_tag, js::value& jv, PkmnDef::PkmnDefPtr const& o)
{
    if (o && !o->id.empty())
    {
        js::array jsTypes;
        for (auto const& e : o->types)
            jsTypes.push_back(js::value(e));
        jv = {
            {"id",    o->id  },
            {"name",  o->name},
            {"types", jsTypes}
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
        auto pkmn  = std::make_shared<PkmnDef>();
        pkmn->id   = id;
        pkmn->name = js::value_to<std::string>(obj.at("name"));
        for (auto& value : obj.at("types").as_array())
            pkmn->types.push_back(js::value_to<std::string>(value));
        return pkmn;
    }
}
