#ifndef GRASSTILE_H
#define GRASSTILE_H

#include "tile.h"

class GrassTile : public Tile
{
public:
    GrassTile(std::string const& spritePath, int row, int col);
};

#endif // GRASSTILE_H
