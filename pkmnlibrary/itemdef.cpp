#include "itemdef.h"

#include <cassert>
#include <sstream>

std::string ItemDef::FieldUseToString(FieldUse e)
{
    switch (e)
    {
    case FieldUse::Direct:
        return "Direct";
    case FieldUse::OnPkmn:
        return "OnPkmn";
    case FieldUse::TR:
        return "TR";
    case FieldUse::HM:
        return "HM";
    case FieldUse::None:
    case FieldUse::__SIZE:
        return "None";
    }
    return "???";
}

ItemDef::FieldUse ItemDef::FieldUseFromString(std::string e)
{
    if (e == "Direct")
        return FieldUse::Direct;
    if (e == "OnPkmn" || e == "OnPokemon")
        return FieldUse::OnPkmn;
    if (e == "TR")
        return FieldUse::TR;
    if (e == "HM")
        return FieldUse::HM;
    return FieldUse::None;
}

std::string ItemDef::BattleUseToString(BattleUse e)
{
    switch (e)
    {
    case BattleUse::Direct:
        return "Direct";
    case BattleUse::OnPkmn:
        return "OnPkmn";
    case BattleUse::OnMove:
        return "OnMove";
    case BattleUse::OnFoe:
        return "OnFoe";
    case BattleUse::OnBattler:
        return "OnBattler";
    case BattleUse::None:
    case BattleUse::__SIZE:
        return "None";
    }
    return "???";
}

ItemDef::BattleUse ItemDef::BattleUseFromString(std::string e)
{
    if (e == "Direct")
        return BattleUse::Direct;
    if (e == "OnPkmn" || e == "OnPokemon")
        return BattleUse::OnPkmn;
    if (e == "OnMove")
        return BattleUse::OnMove;
    if (e == "OnFoe")
        return BattleUse::OnFoe;
    if (e == "OnBattler")
        return BattleUse::OnBattler;
    return BattleUse::None;
}

std::string ItemDef::FlagToString(Flag e)
{
    switch (e)
    {
    case Flag::None:
        return "None";
    case Flag::Fling_10:
        return "Fling_10";
    case Flag::Fling_20:
        return "Fling_20";
    case Flag::Fling_25:
        return "Fling_25";
    case Flag::Fling_30:
        return "Fling_30";
    case Flag::Fling_40:
        return "Fling_40";
    case Flag::Fling_50:
        return "Fling_50";
    case Flag::Fling_55:
        return "Fling_55";
    case Flag::Fling_60:
        return "Fling_60";
    case Flag::Fling_65:
        return "Fling_65";
    case Flag::Fling_70:
        return "Fling_70";
    case Flag::Fling_75:
        return "Fling_75";
    case Flag::Fling_80:
        return "Fling_80";
    case Flag::Fling_85:
        return "Fling_85";
    case Flag::Fling_90:
        return "Fling_90";
    case Flag::Fling_100:
        return "Fling_100";
    case Flag::Fling_110:
        return "Fling_110";
    case Flag::Fling_120:
        return "Fling_120";
    case Flag::Fling_130:
        return "Fling_130";
    case Flag::Fling_150:
        return "Fling_150";
    case Flag::Fling_250:
        return "Fling_250";
    case Flag::Repel:
        return "Repel";
    case Flag::EvolutionStone:
        return "EvolutionStone";
    case Flag::Apricorn:
        return "Apricorn";
    case Flag::Fossil:
        return "Fossil";
    case Flag::Mulch:
        return "Mulch";
    case Flag::TypeGem:
        return "TypeGem";
    case Flag::IconMail:
        return "IconMail";
    case Flag::KeyItem:
        return "KeyItem";
    case Flag::PokeBall:
        return "PokeBall";
    case Flag::Berry:
        return "Berry";
    case Flag::MegaStone:
        return "MegaStone";
    case Flag::MegaRing:
        return "MegaRing";
    case Flag::__SIZE:
        return "__SIZE";
    }
    return "???";
}

ItemDef::Flag ItemDef::FlagFromString(std::string e)
{
    if (e == "Fling_10")
        return Flag::Fling_10;
    if (e == "Fling_20")
        return Flag::Fling_20;
    if (e == "Fling_25")
        return Flag::Fling_25;
    if (e == "Fling_30")
        return Flag::Fling_30;
    if (e == "Fling_40")
        return Flag::Fling_40;
    if (e == "Fling_50")
        return Flag::Fling_50;
    if (e == "Fling_55")
        return Flag::Fling_55;
    if (e == "Fling_60")
        return Flag::Fling_60;
    if (e == "Fling_65")
        return Flag::Fling_65;
    if (e == "Fling_70")
        return Flag::Fling_70;
    if (e == "Fling_75")
        return Flag::Fling_75;
    if (e == "Fling_80")
        return Flag::Fling_80;
    if (e == "Fling_85")
        return Flag::Fling_85;
    if (e == "Fling_90")
        return Flag::Fling_90;
    if (e == "Fling_100")
        return Flag::Fling_100;
    if (e == "Fling_110")
        return Flag::Fling_110;
    if (e == "Fling_120")
        return Flag::Fling_120;
    if (e == "Fling_130")
        return Flag::Fling_130;
    if (e == "Fling_150")
        return Flag::Fling_150;
    if (e == "Fling_250")
        return Flag::Fling_250;
    if (e == "Repel")
        return Flag::Repel;
    if (e == "EvolutionStone")
        return Flag::EvolutionStone;
    if (e == "Apricorn")
        return Flag::Apricorn;
    if (e == "Fossil")
        return Flag::Fossil;
    if (e == "Mulch")
        return Flag::Mulch;
    if (e == "TypeGem")
        return Flag::TypeGem;
    if (e == "IconMail")
        return Flag::IconMail;
    if (e == "KeyItem")
        return Flag::KeyItem;
    if (e == "PokeBall")
        return Flag::PokeBall;
    if (e == "Berry")
        return Flag::Berry;
    if (e == "MegaStone")
        return Flag::MegaStone;
    if (e == "MegaRing")
        return Flag::MegaRing;
    return Flag::None;
}

std::string ItemDef::GetPocketName(size_t pocket)
{
    assert(pocket < NumberOfPockets);

    if (pocket == 0)
        return "Items";
    if (pocket == 1)
        return "Medicine";
    if (pocket == 2)
        return "PkBalls";
    if (pocket == 3)
        return "TMs & HMs";
    if (pocket == 4)
        return "Berries";
    if (pocket == 5)
        return "Mail";
    if (pocket == 6)
        return "Battle Items";
    return "Key Items";
}

ItemDef::ItemDef()
{
}

namespace
{
bool parseBoolLoose(std::string const& s)
{
    return s != "false" && s != "False" && s != "0" && s != "FALSE";
}

void parseNaturalGift(std::string const& token, std::string& typeOut, size_t& powerOut)
{
    // NaturalGift_POISON_80
    static std::string const prefix = "NaturalGift_";
    if (token.rfind(prefix, 0) != 0)
        return;
    auto rest = token.substr(prefix.size());
    auto us   = rest.rfind('_');
    if (us == std::string::npos)
        return;
    typeOut  = rest.substr(0, us);
    powerOut = static_cast<size_t>(std::stoul(rest.substr(us + 1)));
}
} // namespace

ItemDef::ItemDefPtr ItemDef::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto item        = std::make_shared<ItemDef>();
    item->id         = id;
    item->name       = pt.get<std::string>("Name");
    item->namePlural = pt.get<std::string>("NamePlural");
    if (pt.count("PortionName"))
        item->portionName = pt.get<std::string>("PortionName");
    if (pt.count("PortionNamePlural"))
        item->portionNamePlural = pt.get<std::string>("PortionNamePlural");
    item->pocket = pt.get<size_t>("Pocket") - 1;
    item->price  = pt.get<size_t>("Price");
    if (pt.count("BPPrice"))
        item->bpPrice = pt.get<size_t>("BPPrice");
    if (pt.count("FieldUse"))
        item->fieldUse = FieldUseFromString(pt.get<std::string>("FieldUse"));
    if (pt.count("BattleUse"))
        item->battleUse = BattleUseFromString(pt.get<std::string>("BattleUse"));
    if (pt.count("SellPrice"))
        item->sellPrice = pt.get<size_t>("SellPrice");
    if (pt.count("Move"))
        item->moveId = pt.get<std::string>("Move");
    if (pt.count("Consumable"))
        item->consumable = parseBoolLoose(pt.get<std::string>("Consumable"));
    if (pt.count("Flags"))
    {
        std::stringstream ss(pt.get<std::string>("Flags"));
        while (ss.good())
        {
            std::string substr;
            getline(ss, substr, ',');
            if (substr.empty())
                continue;
            if (substr.rfind("NaturalGift_", 0) == 0)
            {
                parseNaturalGift(substr, item->naturalGiftType, item->naturalGiftPower);
                continue;
            }
            Flag f = FlagFromString(substr);
            if (f != Flag::None)
                item->flags.push_back(f);
        }
    }
    item->description = pt.get<std::string>("Description");

    return item;
}

std::vector<ItemDef::ItemDefPtr> ItemDef::vectorFromPropertyTree(pt::ptree const& pt)
{
    std::vector<ItemDefPtr> items;
    for (auto pos = pt.begin(); pos != pt.end(); ++pos)
    {
        auto p = *pos;
        if (p.second.count("Flags"))
        {
            auto flags = p.second.get<std::string>("Flags");
            if (flags.find("MegaStone") != std::string::npos || flags.find("MegaRing") != std::string::npos)
                continue;
        }

        items.push_back(fromPropertyTree(p.first, p.second));
    }
    return items;
}

std::string ItemDef::getId() const
{
    return id;
}

void ItemDef::setId(std::string const& newId)
{
    id = newId;
}

std::string ItemDef::getName() const
{
    return name;
}

std::string ItemDef::getNamePlural() const
{
    return namePlural;
}

std::string ItemDef::getPortionName() const
{
    return portionName;
}

std::string ItemDef::getPortionNamePlural() const
{
    return portionNamePlural;
}

size_t ItemDef::getPocket() const
{
    return pocket;
}

size_t ItemDef::getPrice() const
{
    return price;
}

size_t ItemDef::getBPPrice() const
{
    return bpPrice;
}

ItemDef::FieldUse ItemDef::getFieldUse() const
{
    return fieldUse;
}

ItemDef::BattleUse ItemDef::getBattleUse() const
{
    return battleUse;
}

size_t ItemDef::getSellPrice() const
{
    return sellPrice;
}

size_t ItemDef::effectiveSellPrice() const
{
    return sellPrice > 0 ? sellPrice : price / 2;
}

std::string ItemDef::getMoveId() const
{
    return moveId;
}

std::vector<ItemDef::Flag> ItemDef::getFlags() const
{
    return flags;
}

bool ItemDef::hasFlag(Flag flag) const
{
    for (auto f : flags)
    {
        if (f == flag)
            return true;
    }
    return false;
}

bool ItemDef::isConsumable() const
{
    return consumable;
}

std::string ItemDef::getNaturalGiftType() const
{
    return naturalGiftType;
}

size_t ItemDef::getNaturalGiftPower() const
{
    return naturalGiftPower;
}

std::string ItemDef::getDescription() const
{
    return description;
}

void ItemDef::setName(std::string const& newName)
{
    name = newName;
}

void ItemDef::setNamePlural(std::string const& newNamePlural)
{
    namePlural = newNamePlural;
}

void ItemDef::setPortionName(std::string const& newPortionName)
{
    portionName = newPortionName;
}

void ItemDef::setPortionNamePlural(std::string const& newPortionNamePlural)
{
    portionNamePlural = newPortionNamePlural;
}

void ItemDef::setPocket(size_t newPocket)
{
    pocket = newPocket;
}

void ItemDef::setPrice(size_t newPrice)
{
    price = newPrice;
}

void ItemDef::setBPPrice(size_t newBPPrice)
{
    bpPrice = newBPPrice;
}

void ItemDef::setFieldUse(FieldUse newFieldUse)
{
    fieldUse = newFieldUse;
}

void ItemDef::setBattleUse(BattleUse newBattleUse)
{
    battleUse = newBattleUse;
}

void ItemDef::setSellPrice(size_t newSellPrice)
{
    sellPrice = newSellPrice;
}

void ItemDef::setMoveId(std::string const& newMoveId)
{
    moveId = newMoveId;
}

void ItemDef::setFlags(std::vector<Flag> const& newFlags)
{
    flags = newFlags;
}

void ItemDef::setConsumable(bool newConsumable)
{
    consumable = newConsumable;
}

void ItemDef::setNaturalGiftType(std::string const& newType)
{
    naturalGiftType = newType;
}

void ItemDef::setNaturalGiftPower(size_t newPower)
{
    naturalGiftPower = newPower;
}

void ItemDef::setDescription(std::string const& newDescription)
{
    description = newDescription;
}

void tag_invoke(js::value_from_tag, js::value& jv, ItemDef::ItemDefPtr const& o)
{
    if (o && !o->id.empty())
    {
        js::array jsFlags;
        for (auto const& e : o->flags)
            jsFlags.push_back(js::value(ItemDef::FlagToString(e)));
        jv = {
            {"id",                 o->id                             },
            {"name",               o->name                           },
            {"namePlural",         o->namePlural                     },
            {"portionName",        o->portionName                    },
            {"portionNamePlural",  o->portionNamePlural              },
            {"pocket",             o->pocket                         },
            {"price",              o->price                          },
            {"bpPrice",            o->bpPrice                        },
            {"fieldUse",           static_cast<size_t>(o->fieldUse)  },
            {"battleUse",          static_cast<size_t>(o->battleUse) },
            {"sellPrice",          o->sellPrice                      },
            {"moveId",             o->moveId                         },
            {"flags",              jsFlags                           },
            {"consumable",         o->consumable                     },
            {"naturalGiftType",    o->naturalGiftType                },
            {"naturalGiftPower",   o->naturalGiftPower               },
            {"description",        o->description                    }
        };
    }
    else
    {
        jv = {};
    }
}

ItemDef::ItemDefPtr tag_invoke(js::value_to_tag<ItemDef::ItemDefPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    std::string id = js::value_to<std::string>(obj.at("id"));
    if (id.empty())
        return nullptr;

    auto item        = std::make_shared<ItemDef>();
    item->id         = id;
    item->name       = js::value_to<std::string>(obj.at("name"));
    item->namePlural = js::value_to<std::string>(obj.at("namePlural"));
    if (auto const* v = obj.if_contains("portionName"))
        item->portionName = js::value_to<std::string>(*v);
    if (auto const* v = obj.if_contains("portionNamePlural"))
        item->portionNamePlural = js::value_to<std::string>(*v);
    item->pocket = js::value_to<size_t>(obj.at("pocket"));
    item->price  = js::value_to<size_t>(obj.at("price"));
    if (auto const* v = obj.if_contains("bpPrice"))
        item->bpPrice = js::value_to<size_t>(*v);
    item->fieldUse  = static_cast<ItemDef::FieldUse>(js::value_to<size_t>(obj.at("fieldUse")));
    item->battleUse = static_cast<ItemDef::BattleUse>(js::value_to<size_t>(obj.at("battleUse")));
    item->sellPrice = js::value_to<size_t>(obj.at("sellPrice"));
    item->moveId    = js::value_to<std::string>(obj.at("moveId"));
    if (auto const* fl = obj.if_contains("flags"))
    {
        for (auto const& value : fl->as_array())
        {
            ItemDef::Flag f = ItemDef::FlagFromString(js::value_to<std::string>(value));
            if (f != ItemDef::Flag::None)
                item->flags.push_back(f);
        }
    }
    if (auto const* v = obj.if_contains("consumable"))
        item->consumable = v->as_bool();
    if (auto const* v = obj.if_contains("naturalGiftType"))
        item->naturalGiftType = js::value_to<std::string>(*v);
    if (auto const* v = obj.if_contains("naturalGiftPower"))
        item->naturalGiftPower = js::value_to<size_t>(*v);
    item->description = js::value_to<std::string>(obj.at("description"));
    return item;
}
