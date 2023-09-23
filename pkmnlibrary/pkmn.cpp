#include "pkmn.h"

#include <iostream>

Pkmn::Pkmn()
{
}

Pkmn::PkmnPtr Pkmn::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto pkmn  = std::make_shared<Pkmn>();
    pkmn->id   = id;
    pkmn->name = pt.get<std::string>("Name");

    return pkmn;
}

std::vector<Pkmn::PkmnPtr> Pkmn::vectorFromPropertyTree(pt::ptree const& pt)
{
    std::vector<PkmnPtr> pkmns;
    for (auto pos = pt.begin(); pos != pt.end(); ++pos)
    {
        auto p = *pos;
        pkmns.push_back(fromPropertyTree(p.first, p.second));
    }
    return pkmns;
}

std::string Pkmn::getId() const
{
    return id;
}

std::string Pkmn::getName() const
{
    return name;
}

void Pkmn::setId(std::string const& newId)
{
    id = newId;
}

void Pkmn::setName(std::string const& newName)
{
    name = newName;
}

void tag_invoke(js::value_from_tag, js::value& jv, Pkmn::PkmnPtr const& o)
{
    if (o && !o->id.empty())
    {
        jv = {
            {"id",   o->id  },
            {"name", o->name}
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

    std::string id = js::value_to<std::string>(obj.at("id"));
    if (id.empty())
        return nullptr;
    else
    {
        auto pkmn  = std::make_shared<Pkmn>();
        pkmn->id   = id;
        pkmn->name = js::value_to<std::string>(obj.at("name"));
        return pkmn;
    }
}
