#ifndef TILE_H
#define TILE_H

#include <vector>
#include <memory>
#include <string>

#include <boost/json.hpp>
namespace js = boost::json;

class Tile
{
public:
    Tile(std::string const& spritePath, int row, int col);

    std::string const& getSpritePath() const;
    int                getRow() const;
    int                getCol() const;

    friend void                  tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Tile> const& o);
    friend std::unique_ptr<Tile> tag_invoke(js::value_to_tag<std::unique_ptr<Tile>>, js::value const& jv);

private:
    std::string spritePath;
    int         row = 0;
    int         col = 0;
};

#endif // TILE_H
