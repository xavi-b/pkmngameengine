#ifndef MAP_H
#define MAP_H

#include <vector>
#include <memory>
#include "level.h"

class Map
{
public:
    Map(int nCol, int nRow);

    std::unique_ptr<Level>& addLevel();

    std::vector<std::unique_ptr<Level>>& getLevels();

    int getNCol() const;
    int getNRow() const;

    friend void                 tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Map> const& o);
    friend std::unique_ptr<Map> tag_invoke(js::value_to_tag<std::unique_ptr<Map>>, js::value const& jv);

private:
    int nCol, nRow;

    std::vector<std::unique_ptr<Level>> levels;
};

#endif // MAP_H
