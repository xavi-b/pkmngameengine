#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <memory>
#include "layers/eventlayer.h"
#include "layers/tilelayer.h"

class Level
{
public:
    Level(int nCol, int nRow);

    void addTileLayer(TileLayer::Type type);

    std::vector<std::unique_ptr<TileLayer>>& getTileLayers();

    bool isVisible() const;
    void setVisible(bool value);

private:
    int  nCol, nRow;
    bool visible = true;

    std::vector<std::unique_ptr<TileLayer>> tileLayers;
    std::unique_ptr<EventLayer>             eventLayer;

    // TODO encounters
};

#endif // LEVEL_H
