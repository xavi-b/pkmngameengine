#ifndef TILELAYER_H
#define TILELAYER_H

#include <vector>
#include <memory>
#include <string>
#include "tile.h"
#include "layer.h"

class TileLayer : public Layer<Tile>
{
public:
    enum Type
    {
        GROUND,
        SOLID,
        OVERLAY
    };

    TileLayer(Type type, int nCol, int nRow);

    Type        getType() const;
    std::string getTypeName() const;

protected:
    Type type;
};

#endif // LAYER_H
