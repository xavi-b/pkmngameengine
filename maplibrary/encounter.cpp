#include "encounter.h"

Encounter::Encounter()
{
}

std::string Encounter::getPkmnId() const
{
    return pkmnId;
}

void Encounter::setPkmnId(std::string const& newPkmnId)
{
    pkmnId = newPkmnId;
}

size_t Encounter::getPercentage() const
{
    return percentage;
}

void Encounter::setPercentage(size_t newPercentage)
{
    percentage = newPercentage;
}

size_t Encounter::getLevelMin() const
{
    return levelMin;
}

void Encounter::setLevelMin(size_t newLevelMin)
{
    levelMin = newLevelMin;
}

size_t Encounter::getLevelMax() const
{
    return levelMax;
}

void Encounter::setLevelMax(size_t newLevelMax)
{
    levelMax = newLevelMax;
}

void tag_invoke(js::value_from_tag, js::value& jv, Encounter const& o)
{
    jv = {
        {"pkmnId",     o.pkmnId    },
        {"percentage", o.percentage},
        {"levelMin",   o.levelMin  },
        {"levelMax",   o.levelMax  }
    };
}

Encounter tag_invoke(js::value_to_tag<Encounter>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    Encounter         o;
    o.pkmnId     = js::value_to<std::string>(obj.at("pkmnId"));
    o.percentage = js::value_to<size_t>(obj.at("percentage"));
    if (o.percentage > 100)
        o.percentage = 100;
    o.levelMin = js::value_to<size_t>(obj.at("levelMin"));
    if (o.levelMin < 1)
        o.levelMin = 1;
    if (o.levelMin > 100)
        o.levelMin = 100;
    o.levelMax = js::value_to<size_t>(obj.at("levelMax"));
    if (o.levelMax < 1)
        o.levelMax = 1;
    if (o.levelMax > 100)
        o.levelMax = 100;
    return o;
}
