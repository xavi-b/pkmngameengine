#ifndef TYPE_H
#define TYPE_H

#include <boost/json.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;
namespace pt = boost::property_tree;

class Type
{
public:
    using TypePtr = std::unique_ptr<Type>;

    Type();

    friend void    tag_invoke(js::value_from_tag, js::value& jv, TypePtr const& o);
    friend TypePtr tag_invoke(js::value_to_tag<TypePtr>, js::value const& jv);

    static TypePtr              fromPropertyTree(std::string const& id, pt::ptree const& pt);
    static std::vector<TypePtr> vectorFromPropertyTree(pt::ptree const& pt);

    std::string              getId() const;
    std::string              getName() const;
    std::vector<std::string> getWeaknesses() const;
    std::vector<std::string> getResistances() const;
    std::vector<std::string> getImmunities() const;

private:
    std::string              id;
    std::string              name;
    std::vector<std::string> weaknesses;
    std::vector<std::string> resistances;
    std::vector<std::string> immunities;
};

#endif // TYPE_H
