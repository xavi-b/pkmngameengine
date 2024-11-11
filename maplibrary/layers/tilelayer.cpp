#include "tilelayer.h"

TileLayer::TileLayer(Type type, size_t nCol, size_t nRow) : Layer(nCol, nRow), type(type)
{
}

TileLayer::Type TileLayer::getType() const
{
    return type;
}

std::string TileLayer::getTypeName() const
{
    switch (type)
    {
    case Type::GROUND:
        return "Ground";
    case Type::GROUND_OVERLAY:
        return "Ground Overlay";
    case Type::SOLID:
        return "Solid";
    case Type::SOLID_OVERLAY:
        return "Solid Overlay";
    case Type::OVERLAY:
        return "Overlay";
    default:
        return "Unknown";
    }
}

bool TileLayer::isVisible() const
{
    return visible;
}

void TileLayer::setVisible(bool value)
{
    visible = value;
}

void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<TileLayer> const& o)
{
    jv = {
        {"elements", js::value_from<std::vector<std::vector<Tile::TilePtr>> const&>(o->elements)},
        {"nCol",     o->nCol                                                                    },
        {"nRow",     o->nRow                                                                    },
        {"type",     static_cast<int>(o->type)                                                  }
    };
}

std::unique_ptr<TileLayer> tag_invoke(js::value_to_tag<std::unique_ptr<TileLayer>>, js::value const& jv)
{
    js::object const&          obj = jv.as_object();
    std::unique_ptr<TileLayer> o =
        std::make_unique<TileLayer>(static_cast<TileLayer::Type>(js::value_to<int>(obj.at("type"))),
                                    js::value_to<size_t>(obj.at("nCol")),
                                    js::value_to<size_t>(obj.at("nRow")));
    o->elements = js::value_to<std::vector<std::vector<Tile::TilePtr>>>(obj.at("elements"));
    // for (size_t i = 0; i < o->nCol; ++i)
    // {
    //     if (i < o->elements.size())
    //     {
    //         for (size_t j = o->elements[i].size(); j < o->nRow; ++j)
    //         {
    //             o->elements[i].push_back(std::make_unique<Tile>("", 0, 0));
    //         }
    //     }
    //     else
    //     {
    //         o->elements.push_back(std::vector<Tile::TilePtr>(o->nRow));
    //     }
    // }
    return o;
}
