#include "tile.h"

Tile::Tile(std::string const& spritePath, int row, int col) : spritePath(spritePath), row(row), col(col)
{
}

std::string const& Tile::getSpritePath() const
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

void tag_invoke(js::value_from_tag, js::value& jv, Tile::TilePtr const& o)
{
    if (o)
    {
        jv = {{"spritePath", o->spritePath}, {"col", o->col}, {"row", o->row}};
    }
    else
    {
        jv = {};
    }
}

Tile::TilePtr tag_invoke(js::value_to_tag<Tile::TilePtr>, js::value const& jv)
{
    Tile::TilePtr     o;
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;
    else
        return std::make_unique<Tile>(js::value_to<std::string>(obj.at("spritePath")),
                                      js::value_to<int>(obj.at("row")),
                                      js::value_to<int>(obj.at("col")));
}
