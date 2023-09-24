#ifndef MAP_H
#define MAP_H

#include "encountermethod.h"
#include "level.h"

#include <memory>
#include <vector>

class Map
{
public:
    Map(size_t nCol, size_t nRow);

    std::unique_ptr<Level>& addLevel();

    std::vector<std::unique_ptr<Level>>& getLevels();

    size_t getNCol() const;
    size_t getNRow() const;

    void setNCol(size_t v);
    void setNRow(size_t v);

    std::vector<EncounterMethod> const& getEncounterMethods() const;
    void                                setEncounterMethods(std::vector<EncounterMethod> const& newEncounterMethods);

    friend void                 tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Map> const& o);
    friend std::unique_ptr<Map> tag_invoke(js::value_to_tag<std::unique_ptr<Map>>, js::value const& jv);

private:
    size_t nCol, nRow;

    std::vector<std::unique_ptr<Level>> levels;
    std::vector<EncounterMethod>        encounterMethods;
};

#endif // MAP_H
