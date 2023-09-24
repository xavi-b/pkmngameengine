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

    Tile(std::string const& spritePath, int col, int row);

    std::string const& getSpritePath() const;
    int                getRow() const;
    int                getCol() const;

    friend void    tag_invoke(js::value_from_tag, js::value& jv, TilePtr const& o);
    friend TilePtr tag_invoke(js::value_to_tag<TilePtr>, js::value const& jv);

private:
    std::string spritePath;
    int         col = 0;
    int         row = 0;
};

#endif // TILE_H
