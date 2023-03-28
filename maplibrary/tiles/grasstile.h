#ifndef GRASSTILE_H
#define GRASSTILE_H

#include "tile.h"

class GrassTile : public Tile
{
public:
    GrassTile(const std::string& spritePath, int row, int col);
};

#endif // GRASSTILE_H
