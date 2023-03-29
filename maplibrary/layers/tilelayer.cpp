#include "tilelayer.h"

TileLayer::TileLayer(Type type, int nCol, int nRow) : Layer(nCol, nRow), type(type)
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
