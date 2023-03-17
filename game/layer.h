#ifndef LAYER_H
#define LAYER_H

#include <vector>
#include <memory>
#include "tile.h"

class Layer
{
public:
    Layer();

private:
    std::vector<std::unique_ptr<Tile>> tiles;
};

#endif // LAYER_H
