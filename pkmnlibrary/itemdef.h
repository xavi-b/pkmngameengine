#ifndef ITEMDEF_H
#define ITEMDEF_H

#include <boost/json.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;
namespace pt = boost::property_tree;

class ItemDef
{
public:
    static constexpr size_t NumberOfPockets = 8;

    enum class FieldUse
    {
        None,
        Direct,
        OnPkmn,
        TR,
        HM,
        __SIZE
    };

    enum class BattleUse
    {
        None,
        Direct,
        OnPkmn,
        OnMove,
        OnFoe,
        OnBattler,
        __SIZE
    };

    enum class Flag
    {
        None,
        Fling_10,
        Fling_20,
        Fling_25,
        Fling_30,
        Fling_40,
        Fling_50,
        Fling_60,
        Fling_65,
        Fling_70,
        Fling_75,
        Fling_80,
        Fling_85,
        Fling_90,
        Fling_100,
        Fling_110,
        Fling_120,
        Fling_130,
        Fling_150,
        Fling_250,
        Repel,
        EvolutionStone,
        Apricorn,
        Fossil,
        Mulch,
        TypeGem,
        IconMail,
        KeyItem,
        __SIZE
    };

    static constexpr size_t FieldUseCount = static_cast<size_t>(FieldUse::__SIZE);
    static std::string      FieldUseToString(FieldUse e);
    static FieldUse         FieldUseFromString(std::string e);

    static constexpr size_t BattleUseCount = static_cast<size_t>(BattleUse::__SIZE);
    static std::string      BattleUseToString(BattleUse e);
    static BattleUse        BattleUseFromString(std::string e);

    static constexpr size_t FlagCount = static_cast<size_t>(Flag::__SIZE);
    static std::string      FlagToString(Flag e);
    static Flag             FlagFromString(std::string e);

    static std::string GetPocketName(size_t pocket);

    using ItemDefPtr = std::shared_ptr<ItemDef>;

    ItemDef();

    friend void       tag_invoke(js::value_from_tag, js::value& jv, ItemDefPtr const& o);
    friend ItemDefPtr tag_invoke(js::value_to_tag<ItemDefPtr>, js::value const& jv);

    static ItemDefPtr              fromPropertyTree(std::string const& id, pt::ptree const& pt);
    static std::vector<ItemDefPtr> vectorFromPropertyTree(pt::ptree const& pt);

    std::string getId() const;
    void        setId(std::string const& newId);

    std::string       getName() const;
    std::string       getNamePlural() const;
    size_t            getPocket() const;
    size_t            getPrice() const;
    FieldUse          getFieldUse() const;
    BattleUse         getBattleUse() const;
    size_t            getSellPrice() const;
    std::string       getMoveId() const;
    std::vector<Flag> getFlags() const;
    std::string       getDescription() const;

    void setName(std::string const& newName);
    void setNamePlural(std::string const& newNamePlural);
    void setPocket(size_t newPocket);
    void setPrice(size_t newPrice);
    void setFieldUse(FieldUse newFieldUse);
    void setBattleUse(BattleUse newBattleUse);
    void setSellPrice(size_t newSellPrice);
    void setMoveId(std::string const& newMoveId);
    void setFlags(std::vector<Flag> const& newFlags);
    void setDescription(std::string const& newDescription);

private:
    std::string       id;
    std::string       name;
    std::string       namePlural;
    size_t            pocket    = 0; // / NumberOfPockets
    size_t            price     = 0;
    FieldUse          fieldUse  = FieldUse::None;
    BattleUse         battleUse = BattleUse::None;
    size_t            sellPrice = 0;
    std::string       moveId;
    std::vector<Flag> flags;
    std::string       description;
};

#endif // ITEMDEF_H
