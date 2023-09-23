#ifndef PKMN_H
#define PKMN_H

#include <boost/json.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;
namespace pt = boost::property_tree;

class Pkmn
{
public:
    using PkmnPtr = std::shared_ptr<Pkmn>;

    Pkmn();

    friend void    tag_invoke(js::value_from_tag, js::value& jv, PkmnPtr const& o);
    friend PkmnPtr tag_invoke(js::value_to_tag<PkmnPtr>, js::value const& jv);

    static PkmnPtr              fromPropertyTree(std::string const& id, pt::ptree const& pt);
    static std::vector<PkmnPtr> vectorFromPropertyTree(pt::ptree const& pt);

    std::string getId() const;
    std::string getName() const;

    void setId(std::string const& newId);
    void setName(std::string const& newName);

private:
    std::string id;
    std::string name;
};

#endif // PKMN_H
