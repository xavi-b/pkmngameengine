#ifndef PKMNDEF_H
#define PKMNDEF_H

#include <boost/json.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;
namespace pt = boost::property_tree;

class PkmnDef
{
public:
    using PkmnDefPtr = std::shared_ptr<PkmnDef>;

    PkmnDef();

    friend void       tag_invoke(js::value_from_tag, js::value& jv, PkmnDefPtr const& o);
    friend PkmnDefPtr tag_invoke(js::value_to_tag<PkmnDefPtr>, js::value const& jv);

    static PkmnDefPtr              fromPropertyTree(std::string const& id, pt::ptree const& pt);
    static std::vector<PkmnDefPtr> vectorFromPropertyTree(pt::ptree const& pt);

    std::string              getId() const;
    std::string              getName() const;
    std::vector<std::string> getTypes() const;

    void setId(std::string const& newId);
    void setName(std::string const& newName);
    void setTypes(std::vector<std::string> const& newTypes);

private:
    std::string              id;
    std::string              name;
    std::vector<std::string> types;
};

#endif // PKMNDEF_H
