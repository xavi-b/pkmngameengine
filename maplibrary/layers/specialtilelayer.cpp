#include "specialtilelayer.h"

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, SpecialTileTypePtr const& o)
{
    if (o)
    {
        jv = *o.get();
    }
    else
    {
        jv = 0;
    }
}

SpecialTileTypePtr tag_invoke(js::value_to_tag<SpecialTileTypePtr>, boost::json::value const& jv)
{
    size_t v = js::value_to<size_t>(jv);
    if (v < SpecialTileType::GRASS)
        return nullptr;
    return std::make_unique<SpecialTileType>(static_cast<SpecialTileType>(v));
}

SpecialTileLayer::SpecialTileLayer(size_t nCol, size_t nRow) : Layer<SpecialTileTypePtr>(nCol, nRow)
{
}

std::string SpecialTileTypeToString(SpecialTileType e)
{
    switch (e)
    {
    case GRASS:
        return "LAND";
    case WATER:
        return "WATER";
    case TREE:
        return "TREE";
    case ROCK:
        return "ROCK";
    case BOULDER:
        return "BOULDER";
    case CASCADE:
        return "CASCADE";
    case __SIZE:
        return "__SIZE";
    }
    return "???";
}

void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<SpecialTileLayer> const& o)
{
    jv = {
        {"elements", js::value_from<std::vector<std::vector<SpecialTileTypePtr>> const&>(o->elements)},
        {"nCol",     o->nCol                                                                         },
        {"nRow",     o->nRow                                                                         }
    };
}

std::unique_ptr<SpecialTileLayer> tag_invoke(js::value_to_tag<std::unique_ptr<SpecialTileLayer>>, js::value const& jv)
{
    js::object const&                 obj = jv.as_object();
    std::unique_ptr<SpecialTileLayer> o =
        std::make_unique<SpecialTileLayer>(js::value_to<size_t>(obj.at("nCol")), js::value_to<size_t>(obj.at("nRow")));

    o->elements = js::value_to<std::vector<std::vector<SpecialTileTypePtr>>>(obj.at("elements"));

    return o;
}

SpecialTileTypePtr clone(SpecialTileTypePtr const& other)
{
    return SpecialTileTypePtr(new SpecialTileType(*other.get()));
}
