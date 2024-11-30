#include "itemdef.h"

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
    else if (e == "OnPkmn" || e == "OnPokemon")
        return FieldUse::OnPkmn;
    else if (e == "TR")
        return FieldUse::TR;
    else if (e == "HM")
        return FieldUse::HM;
    else
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
    else if (e == "OnPkmn" || e == "OnPokemon")
        return BattleUse::OnPkmn;
    else if (e == "OnMove")
        return BattleUse::OnMove;
    else if (e == "OnFoe")
        return BattleUse::OnFoe;
    else if (e == "OnBattler")
        return BattleUse::OnBattler;
    else
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
    case Flag::__SIZE:
        return "__SIZE";
        break;
    }
    return "???";
}

ItemDef::Flag ItemDef::FlagFromString(std::string e)
{
    if (e == "Fling_10")
        return Flag::Fling_10;
    else if (e == "Fling_20")
        return Flag::Fling_20;
    else if (e == "Fling_25")
        return Flag::Fling_25;
    else if (e == "Fling_30")
        return Flag::Fling_30;
    else if (e == "Fling_40")
        return Flag::Fling_40;
    else if (e == "Fling_50")
        return Flag::Fling_50;
    else if (e == "Fling_60")
        return Flag::Fling_60;
    else if (e == "Fling_65")
        return Flag::Fling_65;
    else if (e == "Fling_70")
        return Flag::Fling_70;
    else if (e == "Fling_75")
        return Flag::Fling_75;
    else if (e == "Fling_80")
        return Flag::Fling_80;
    else if (e == "Fling_85")
        return Flag::Fling_85;
    else if (e == "Fling_90")
        return Flag::Fling_90;
    else if (e == "Fling_100")
        return Flag::Fling_100;
    else if (e == "Fling_110")
        return Flag::Fling_110;
    else if (e == "Fling_120")
        return Flag::Fling_120;
    else if (e == "Fling_130")
        return Flag::Fling_130;
    else if (e == "Fling_150")
        return Flag::Fling_150;
    else if (e == "Fling_250")
        return Flag::Fling_250;
    else if (e == "Repel")
        return Flag::Repel;
    else if (e == "EvolutionStone")
        return Flag::EvolutionStone;
    else if (e == "Apricorn")
        return Flag::Apricorn;
    else if (e == "Fossil")
        return Flag::Fossil;
    else if (e == "Mulch")
        return Flag::Mulch;
    else if (e == "TypeGem")
        return Flag::TypeGem;
    else if (e == "IconMail")
        return Flag::IconMail;
    else if (e == "KeyItem")
        return Flag::KeyItem;
    else
        return Flag::None;
}

std::string ItemDef::GetPocketName(size_t pocket)
{
    assert(pocket < NumberOfPockets);

    if (pocket == 0)
        return "Items";
    else if (pocket == 1)
        return "Medicine";
    else if (pocket == 2)
        return "PkBalls";
    else if (pocket == 3)
        return "TMs & HMs";
    else if (pocket == 4)
        return "Berries";
    else if (pocket == 5)
        return "Mail";
    else if (pocket == 6)
        return "Battle Items";
    else
        return "Key Items";
}

ItemDef::ItemDef()
{
}

ItemDef::ItemDefPtr ItemDef::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto item        = std::make_shared<ItemDef>();
    item->id         = id;
    item->name       = pt.get<std::string>("Name");
    item->namePlural = pt.get<std::string>("NamePlural");
    item->pocket     = pt.get<size_t>("Pocket") - 1;
    item->price      = pt.get<size_t>("Price");
    if (pt.count("FieldUse"))
        item->fieldUse = FieldUseFromString(pt.get<std::string>("FieldUse"));
    if (pt.count("BattleUse"))
        item->battleUse = BattleUseFromString(pt.get<std::string>("BattleUse"));
    if (pt.count("SellPrice"))
        item->sellPrice = pt.get<size_t>("SellPrice");
    if (pt.count("Move"))
        item->moveId = pt.get<std::string>("Move");
    if (pt.count("Flags"))
    {
        std::stringstream ss(pt.get<std::string>("Flags"));
        while (ss.good())
        {
            std::string substr;
            getline(ss, substr, ',');
            item->flags.push_back(FlagFromString(substr));
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

size_t ItemDef::getPocket() const
{
    return pocket;
}

size_t ItemDef::getPrice() const
{
    return price;
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

std::string ItemDef::getMoveId() const
{
    return moveId;
}

std::vector<ItemDef::Flag> ItemDef::getFlags() const
{
    return flags;
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

void ItemDef::setPocket(size_t newPocket)
{
    pocket = newPocket;
}

void ItemDef::setPrice(size_t newPrice)
{
    price = newPrice;
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
            jsFlags.push_back(ItemDef::FlagToString(e).c_str());
        jv = {
            {"id",          o->id                            },
            {"name",        o->name                          },
            {"namePlural",  o->namePlural                    },
            {"pocket",      o->pocket                        },
            {"price",       o->price                         },
            {"fieldUse",    static_cast<size_t>(o->fieldUse) },
            {"battleUse",   static_cast<size_t>(o->battleUse)},
            {"sellPrice",   o->sellPrice                     },
            {"moveId",      o->moveId                        },
            {"flags",       jsFlags                          },
            {"description", o->description                   }
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
    else
    {
        auto item        = std::make_shared<ItemDef>();
        item->id         = id;
        item->name       = js::value_to<std::string>(obj.at("name"));
        item->namePlural = js::value_to<std::string>(obj.at("namePlural"));
        item->pocket     = js::value_to<size_t>(obj.at("pocket"));
        item->price      = js::value_to<size_t>(obj.at("price"));
        item->fieldUse   = static_cast<ItemDef::FieldUse>(js::value_to<size_t>(obj.at("fieldUse")));
        item->battleUse  = static_cast<ItemDef::BattleUse>(js::value_to<size_t>(obj.at("battleUse")));
        item->sellPrice  = js::value_to<size_t>(obj.at("sellPrice"));
        item->moveId     = js::value_to<std::string>(obj.at("moveId"));
        for (auto& value : obj.at("flags").as_array())
            item->flags.push_back(ItemDef::FlagFromString(js::value_to<std::string>(value)));
        item->description = js::value_to<std::string>(obj.at("description"));
        return item;
    }
}
