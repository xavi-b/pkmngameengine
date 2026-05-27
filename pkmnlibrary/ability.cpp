#include "ability.h"

#include <algorithm>

Ability::Ability()
{
}

Ability::AbilityPtr Ability::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto ability = std::make_shared<Ability>();
    ability->id  = id;

    ability->name        = pt.get<std::string>("Name", "");
    ability->description = pt.get<std::string>("Description", "");

    return ability;
}

std::vector<Ability::AbilityPtr> Ability::vectorFromPropertyTree(pt::ptree const& pt)
{
    std::vector<AbilityPtr> abilities;
    for (auto pos = pt.begin(); pos != pt.end(); ++pos)
    {
        auto p = *pos;
        abilities.push_back(fromPropertyTree(p.first, p.second));
    }
    return abilities;
}

std::string Ability::getId() const
{
    return id;
}

void Ability::setId(std::string const& newId)
{
    id = newId;
}

std::string Ability::getName() const
{
    return name;
}

void Ability::setName(std::string const& newName)
{
    name = newName;
}

std::string Ability::getDescription() const
{
    return description;
}

void Ability::setDescription(std::string const& newDescription)
{
    description = newDescription;
}

void tag_invoke(js::value_from_tag, js::value& jv, Ability::AbilityPtr const& o)
{
    if (o && !o->id.empty())
    {
        jv = {
            {"id",          o->id         },
            {"name",        o->name       },
            {"description", o->description}
        };
    }
    else
    {
        jv = {};
    }
}

Ability::AbilityPtr tag_invoke(js::value_to_tag<Ability::AbilityPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    std::string id = js::value_to<std::string>(obj.at("id"));
    if (id.empty())
        return nullptr;

    auto ability         = std::make_shared<Ability>();
    ability->id          = id;
    ability->name        = js::value_to<std::string>(obj.at("name"));
    ability->description = js::value_to<std::string>(obj.at("description"));
    return ability;
}
