#include "tile.h"

Tile::Tile(std::string const& spritePath, int col, int row) : spritePath(spritePath), col(col), row(row)
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
    if (o && !o->spritePath.empty())
    {
        jv = {
            {"spritePath", o->spritePath},
            {"col",        o->col       },
            {"row",        o->row       }
        };
    }
    else
    {
        jv = {};
    }
}

Tile::TilePtr tag_invoke(js::value_to_tag<Tile::TilePtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    std::string spritePath = js::value_to<std::string>(obj.at("spritePath"));
    if (spritePath.empty())
        return nullptr;
    else
        return std::make_unique<Tile>(spritePath,
                                      js::value_to<size_t>(obj.at("col")),
                                      js::value_to<size_t>(obj.at("row")));
}
