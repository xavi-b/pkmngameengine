#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <memory>
#include "layers/tilelayer.h"
#include "layers/eventlayer.h"
#include "layers/encounterslayer.h"

class Level
{
public:
    Level(int nCol, int nRow);

    void addTileLayer(TileLayer::Type type);

    std::vector<std::unique_ptr<TileLayer>>& getTileLayers();

    bool isVisible() const;
    void setVisible(bool value);

    friend void                   tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Level> const& o);
    friend std::unique_ptr<Level> tag_invoke(js::value_to_tag<std::unique_ptr<Level>>, js::value const& jv);

private:
    int  nCol, nRow;
    bool visible = true;

    std::vector<std::unique_ptr<TileLayer>> tileLayers;
    std::unique_ptr<EventLayer>             eventLayer;
    std::unique_ptr<EncountersLayer>        encountersLayer;
};

#endif // LEVEL_H
