#include "tilelayer.h"

TileLayer::TileLayer(Type type) : Layer(), type(type)
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
