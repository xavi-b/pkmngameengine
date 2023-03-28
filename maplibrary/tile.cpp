#include "tile.h"

Tile::Tile(const std::string& spritePath, int row, int col) : spritePath(spritePath), row(row), col(col)
{
}

const std::string& Tile::getSpritePath() const
{
    return spritePath;
}

int Tile::getRow() const
{
    return row;
}

int Tile::getCol() const
{
    return col;
}

void Tile::callEnterEvent(int param)
{
}

void Tile::callInterractionEvent(int param)
{
}
