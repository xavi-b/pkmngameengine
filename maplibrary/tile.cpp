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

void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Tile> const& o)
{
    jv = {{"spritePath", o->spritePath}, {"col", o->col}, {"row", o->row}};
}

std::unique_ptr<Tile> tag_invoke(js::value_to_tag<std::unique_ptr<Tile>>, js::value const& jv)
{
    std::unique_ptr<Tile> o;
    js::object const&     obj = jv.as_object();
    return std::make_unique<Tile>(js::value_to<std::string>(obj.at("spritePath")),
                                  js::value_to<int>(obj.at("row")),
                                  js::value_to<int>(obj.at("col")));
}
