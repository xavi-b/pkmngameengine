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
    case GROUND:
        return "Ground";
    case SOLID:
        return "Solid";
    case OVERLAY:
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
        {"type",     o->type                                                                    }
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
    return o;
}
