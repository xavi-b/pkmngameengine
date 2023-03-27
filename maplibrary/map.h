#ifndef MAP_H
#define MAP_H

#include <vector>
#include <memory>
#include "layers/eventlayer.h"
#include "layers/tilelayer.h"

class Map
{
public:
    Map();

    void addTileLayer(TileLayer::Type type);

    std::vector<std::unique_ptr<TileLayer>>& getTileLayers();

private:
    std::vector<std::unique_ptr<TileLayer>> tileLayers;
    std::unique_ptr<EventLayer>             eventLayer;

    // TODO encounters
};

#endif // MAP_H
