#include "level.h"

Level::Level(int nCol, int nRow) : nCol(nCol), nRow(nRow)
{
    addTileLayer(TileLayer::Type::GROUND);
    addTileLayer(TileLayer::Type::SOLID);
    addTileLayer(TileLayer::Type::OVERLAY);
}

void Level::addTileLayer(TileLayer::Type type)
{
    tileLayers.emplace_back(std::make_unique<TileLayer>(type, nCol, nRow));
}

std::vector<std::unique_ptr<TileLayer>>& Level::getTileLayers()
{
    return tileLayers;
}

bool Level::isVisible() const
{
    return visible;
}

void Level::setVisible(bool value)
{
    visible = value;
}
