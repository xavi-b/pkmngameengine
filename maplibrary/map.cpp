#include "map.h"

Map::Map(int nCol, int nRow) : nCol(nCol), nRow(nRow)
{
    levels.emplace_back(std::make_unique<Level>(nCol, nRow));
}

std::unique_ptr<Level>& Map::addLevel()
{
    levels.emplace_back(std::make_unique<Level>(nCol, nRow));
    return levels.back();
}

std::vector<std::unique_ptr<Level>>& Map::getLevels()
{
    return levels;
}
