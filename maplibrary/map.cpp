#include "map.h"

Map::Map(int nCol, int nRow) : nCol(nCol), nRow(nRow)
{
}

void Map::addTileLayer(TileLayer::Type type)
{
    tileLayers.emplace_back(std::make_unique<TileLayer>(type, nCol, nRow));
}

std::vector<std::unique_ptr<TileLayer>>& Map::getTileLayers()
{
    return tileLayers;
}
