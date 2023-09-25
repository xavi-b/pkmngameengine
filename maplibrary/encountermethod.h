#ifndef ENCOUNTERMETHOD_H
#define ENCOUNTERMETHOD_H

#include "encounter.h"

#include <boost/json.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;

class EncounterMethod
{
public:
    enum Type
    {
        LAND,
        LAND_NIGHT,
        LAND_MORNING,
        CAVE,
        WATER,
        OLD_ROD,
        GOOD_ROD,
        SUPER_ROD,
        ROCK_SMASH,
        CUT,
        HEADBUTT_LOW,
        HEADBUTT_HIGH,
        BUG_CONTEST,
        POKE_RADAR,
        __SIZE
    };

    static constexpr size_t TypeCount = Type::__SIZE + Type::LAND;
    static std::string      TypeToString(Type e);

    EncounterMethod(Type type);

    friend void            tag_invoke(js::value_from_tag, js::value& jv, EncounterMethod const& o);
    friend EncounterMethod tag_invoke(js::value_to_tag<EncounterMethod>, js::value const& jv);

    Type                   getType() const;
    void                   setType(Type newType);
    size_t                 getDensity() const;
    void                   setDensity(size_t newDensity);
    std::vector<Encounter> getEncounters() const;
    void                   setEncounters(std::vector<Encounter> const& newEncounters);

private:
    Type                   type;
    size_t                 density = 0;
    std::vector<Encounter> encounters;
};

#endif // ENCOUNTERMETHOD_H
