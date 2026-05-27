#ifndef ABILITY_H
#define ABILITY_H

#include <boost/json.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;
namespace pt = boost::property_tree;

class Ability
{
public:
    using AbilityPtr = std::shared_ptr<Ability>;

    Ability();

    friend void       tag_invoke(js::value_from_tag, js::value& jv, AbilityPtr const& o);
    friend AbilityPtr tag_invoke(js::value_to_tag<AbilityPtr>, js::value const& jv);

    static AbilityPtr              fromPropertyTree(std::string const& id, pt::ptree const& pt);
    static std::vector<AbilityPtr> vectorFromPropertyTree(pt::ptree const& pt);

    std::string getId() const;
    void        setId(std::string const& newId);

    std::string getName() const;
    void        setName(std::string const& newName);

    std::string getDescription() const;
    void        setDescription(std::string const& newDescription);

private:
    std::string id;
    std::string name;
    std::string description;
};

#endif // ABILITY_H
