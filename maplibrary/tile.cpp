#include "tile.h"

Tile::Tile(std::string const& spritePath, size_t col, size_t row) : spritePath(spritePath), col(col), row(row)
{
}

std::string const& Tile::getSpritePath() const
{
    return spritePath;
}

size_t Tile::getRow() const
{
    return row;
}

void Tile::setRow(size_t newRow)
{
    row = newRow;
}

size_t Tile::getCol() const
{
    return col;
}

void Tile::setCol(size_t newCol)
{
    col = newCol;
}

bool Tile::isAnimated() const
{
    return animated;
}

void Tile::setAnimated(bool newAnimated)
{
    animated = newAnimated;
}

bool Tile::isDoor() const
{
    return door;
}

void Tile::setDoor(bool newDoor)
{
    door = newDoor;
}

void tag_invoke(js::value_from_tag, js::value& jv, Tile::TilePtr const& o)
{
    if (o && !o->spritePath.empty())
    {
        jv = {
            {"spritePath", o->spritePath},
            {"col",        o->col       },
            {"row",        o->row       },
            {"animated",   o->animated  },
            {"door",       o->door      }
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
    {
        auto tile = std::make_unique<Tile>(spritePath,
                                           js::value_to<size_t>(obj.at("col")),
                                           js::value_to<size_t>(obj.at("row")));
        if (obj.contains("animated"))
            tile->setAnimated(js::value_to<bool>(obj.at("animated")));
        if (obj.contains("door"))
            tile->setDoor(js::value_to<bool>(obj.at("door")));
        return tile;
    }
}

Tile::TilePtr clone(Tile::TilePtr const& other)
{
    return Tile::TilePtr(new Tile(*other.get()));
}
