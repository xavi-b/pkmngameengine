#include "map.h"

Map::Map()
{
}

void Map::addTileLayer(TileLayer::Type type)
{
    tileLayers.emplace_back(std::make_unique<TileLayer>(type));
}

std::vector<std::unique_ptr<TileLayer>>& Map::getTileLayers()
{
    return tileLayers;
}
