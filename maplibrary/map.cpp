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

int Map::getNCol() const
{
    return nCol;
}

int Map::getNRow() const
{
    return nRow;
}

void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Map> const& o)
{
    jv = {{"nCol", o->nCol},
          {"nRow", o->nRow},
          {"levels", js::value_from<std::vector<std::unique_ptr<Level>> const&>(o->levels)}};
}

std::unique_ptr<Map> tag_invoke(js::value_to_tag<std::unique_ptr<Map>>, js::value const& jv)
{
    js::object const&    obj = jv.as_object();
    std::unique_ptr<Map> o =
        std::make_unique<Map>(js::value_to<int>(obj.at("nCol")), js::value_to<int>(obj.at("nRow")));

    o->levels = js::value_to<std::vector<std::unique_ptr<Level>>>(obj.at("levels"));

    return o;
}
