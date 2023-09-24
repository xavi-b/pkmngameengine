#ifndef ENCOUNTER_H
#define ENCOUNTER_H

#include <boost/json.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;

class Encounter
{
public:
    Encounter();

    friend void      tag_invoke(js::value_from_tag, js::value& jv, Encounter const& o);
    friend Encounter tag_invoke(js::value_to_tag<Encounter>, js::value const& jv);

    std::string getPkmnId() const;
    void        setPkmnId(std::string const& newPkmnId);
    size_t      getPercentage() const;
    void        setPercentage(size_t newPercentage);
    size_t      getLevelMin() const;
    void        setLevelMin(size_t newLevelMin);
    size_t      getLevelMax() const;
    void        setLevelMax(size_t newLevelMax);

private:
    std::string pkmnId;
    size_t      percentage;
    size_t      levelMin;
    size_t      levelMax;
};

#endif // ENCOUNTER_H
