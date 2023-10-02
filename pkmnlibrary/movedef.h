#ifndef MOVEDEF_H
#define MOVEDEF_H

#include <boost/json.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;
namespace pt = boost::property_tree;

class MoveDef
{
public:
    enum Category
    {
        PHYSICAL,
        SPECIAL,
        STATUS
    };

    static std::string CategoryToString(Category e);
    static Category    CategoryFromString(std::string e);

    using MoveDefPtr = std::shared_ptr<MoveDef>;

    MoveDef();

    friend void       tag_invoke(js::value_from_tag, js::value& jv, MoveDefPtr const& o);
    friend MoveDefPtr tag_invoke(js::value_to_tag<MoveDefPtr>, js::value const& jv);

    static MoveDefPtr              fromPropertyTree(std::string const& id, pt::ptree const& pt);
    static std::vector<MoveDefPtr> vectorFromPropertyTree(pt::ptree const& pt);

    std::string getId() const;
    void        setId(std::string const& newId);

    std::string getName() const;
    void        setName(std::string const& newName);

    std::string getType() const;
    void        setType(std::string const& newType);

    size_t getAccuracy() const;
    void   setAccuracy(size_t newAccuracy);

    size_t getPower() const;
    void   setPower(size_t newPower);

    size_t getTotalPP() const;
    void   setTotalPP(size_t newTotalPP);

    Category getCategory() const;
    void     setCategory(Category newCategory);

private:
    std::string id;
    std::string name;
    std::string type;
    size_t      accuracy = 0;
    size_t      power    = 0;
    size_t      totalPP  = 0;
    Category    category = PHYSICAL;
};

#endif // MOVEDEF_H
