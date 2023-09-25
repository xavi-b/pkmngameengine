#include "encountermethod.h"

std::string EncounterMethod::TypeToString(Type e)
{
    switch (e)
    {
    case LAND:
        return "LAND";
    case LAND_NIGHT:
        return "LAND_NIGHT";
    case LAND_MORNING:
        return "LAND_MORNING";
    case CAVE:
        return "CAVE";
    case WATER:
        return "WATER";
    case OLD_ROD:
        return "OLD_ROD";
    case GOOD_ROD:
        return "GOOD_ROD";
    case SUPER_ROD:
        return "SUPER_ROD";
    case ROCK_SMASH:
        return "ROCK_SMASH";
    case CUT:
        return "CUT";
    case HEADBUTT_LOW:
        return "HEADBUTT_LOW";
    case HEADBUTT_HIGH:
        return "HEADBUTT_HIGH";
    case BUG_CONTEST:
        return "BUG_CONTEST";
    case POKE_RADAR:
        return "POKE_RADAR";
    default:
        return "???";
    }
}

EncounterMethod::EncounterMethod(Type type) : type(type)
{
}

EncounterMethod::Type EncounterMethod::getType() const
{
    return type;
}

void EncounterMethod::setType(Type newType)
{
    type = newType;
}

size_t EncounterMethod::getDensity() const
{
    return density;
}

void EncounterMethod::setDensity(size_t newDensity)
{
    density = newDensity;
}

std::vector<Encounter> EncounterMethod::getEncounters() const
{
    return encounters;
}

void EncounterMethod::setEncounters(std::vector<Encounter> const& newEncounters)
{
    encounters = newEncounters;
}

void tag_invoke(js::value_from_tag, js::value& jv, EncounterMethod const& o)
{
    jv = {
        {"type",       static_cast<size_t>(o.type)                                },
        {"density",    o.density                                                  },
        {"encounters", js::value_from<std::vector<Encounter> const&>(o.encounters)}
    };
}

EncounterMethod tag_invoke(js::value_to_tag<EncounterMethod>, js::value const& jv)
{
    js::object const& obj = jv.as_object();

    EncounterMethod::Type type = static_cast<EncounterMethod::Type>(js::value_to<size_t>(obj.at("type")));
    EncounterMethod       o(type);
    o.density = js::value_to<size_t>(obj.at("density"));
    if (o.density > 100)
        o.density = 100;
    o.encounters = js::value_to<std::vector<Encounter>>(obj.at("encounters"));
    return o;
}
