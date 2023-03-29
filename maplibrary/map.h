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

private:
    int nCol, nRow;

    std::vector<std::unique_ptr<Level>> levels;

    // TODO encounters
};

#endif // MAP_H
