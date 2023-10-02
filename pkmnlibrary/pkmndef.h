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

    std::vector<std::string> getTypes() const;
    void                     setTypes(std::vector<std::string> const& newTypes);

    std::map<int, std::string> getMovesToLearn() const;
    void                       setMovesToLearn(std::map<int, std::string> const& newMovesToLearn);

private:
    std::string                id;
    std::string                name;
    std::vector<std::string>   types;
    std::map<int, std::string> movesToLearn;
};

#endif // PKMNDEF_H
