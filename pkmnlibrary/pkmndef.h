#ifndef PKMNDEF_H
#define PKMNDEF_H

#include <boost/json.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;
namespace pt = boost::property_tree;

class PkmnDef
{
public:
    enum Stat
    {
        HP,
        ATTACK,
        DEFENSE,
        SPECIAL_ATTACK,
        SPECIAL_DEFENSE,
        SPEED,
        __SIZE_STAT
    };

    static constexpr size_t StatCount = Stat::__SIZE_STAT;
    static std::string      StatToString(Stat e);
    static Stat             StatFromString(std::string const& e);

    enum GrowthRate
    {
        ERRATIC,
        FAST,
        MEDIUM_FAST, // PARABOLIC
        MEDIUM_SLOW, // MEDIUM
        SLOW,
        FLUCTUATING,
        __SIZE_GROWTH_RATE
    };

    static constexpr size_t GrowthRateCount = GrowthRate::__SIZE_GROWTH_RATE;
    static std::string      GrowthRateToString(GrowthRate e);
    static GrowthRate       GrowthRateFromString(std::string const& e);

    using PkmnDefPtr = std::shared_ptr<PkmnDef>;

    PkmnDef();

    friend void       tag_invoke(js::value_from_tag, js::value& jv, PkmnDefPtr const& o);
    friend PkmnDefPtr tag_invoke(js::value_to_tag<PkmnDefPtr>, js::value const& jv);

    static PkmnDefPtr              fromPropertyTree(std::string const& id, pt::ptree const& pt);
    static std::vector<PkmnDefPtr> vectorFromPropertyTree(pt::ptree const& pt);

    std::string getId() const;
    void        setId(std::string const& newId);

    std::string getName() const;
    void        setName(std::string const& newName);

    std::vector<std::string> const& getTypes() const;
    void                            setTypes(std::vector<std::string> const& newTypes);

    std::map<int, std::string> const& getMovesToLearn() const;
    void                              setMovesToLearn(std::map<int, std::string> const& newMovesToLearn);

    std::map<PkmnDef::Stat, size_t> const& getBaseStats() const;
    void                                   setBaseStats(std::map<PkmnDef::Stat, size_t> const& newBaseStats);

    std::map<PkmnDef::Stat, size_t> const& getEVsToLearn() const;
    void                                   setEVsToLearn(std::map<PkmnDef::Stat, size_t> const& newEVsToLearn);

private:
    std::string                     id;
    std::string                     name;
    std::vector<std::string>        types;
    std::map<int, std::string>      movesToLearn;
    std::map<PkmnDef::Stat, size_t> baseStats;
    std::map<PkmnDef::Stat, size_t> EVsToLearn;
    GrowthRate                      growthRate = GrowthRate::MEDIUM_SLOW;
};

#endif // PKMNDEF_H
