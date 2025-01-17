#ifndef TILE_H
#define TILE_H

#include <boost/json.hpp>
#include <memory>
#include <string>
#include <vector>
namespace js = boost::json;

class Tile
{
public:
    using TilePtr = std::unique_ptr<Tile>;

    Tile(std::string const& spritePath, size_t col, size_t row);

    std::string const& getSpritePath() const;
    size_t             getRow() const;
    size_t             getCol() const;

    bool isAnimated() const;
    void setAnimated(bool newAnimated);
    bool isDoor() const;
    void setDoor(bool newDoor);

    friend void    tag_invoke(js::value_from_tag, js::value& jv, TilePtr const& o);
    friend TilePtr tag_invoke(js::value_to_tag<TilePtr>, js::value const& jv);

    friend TilePtr clone(TilePtr const& other);

private:
    std::string spritePath;
    size_t      col      = 0;
    size_t      row      = 0;
    bool        animated = false;
    bool        door     = false;
};

#endif // TILE_H
