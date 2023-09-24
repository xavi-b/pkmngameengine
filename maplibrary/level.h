#ifndef LEVEL_H
#define LEVEL_H

#include "layers/eventlayer.h"
#include "layers/specialtilelayer.h"
#include "layers/tilelayer.h"

#include <memory>
#include <vector>

class Level
{
public:
    Level(size_t nCol, size_t nRow);

    void addTileLayer(TileLayer::Type type);

    std::vector<std::unique_ptr<TileLayer>>& getTileLayers();
    std::unique_ptr<EventLayer>&             getEventLayer();
    std::unique_ptr<SpecialTileLayer>&       getSpecialTileLayer();

    bool isVisible() const;
    void setVisible(bool value);

    void setNCol(size_t v);
    void setNRow(size_t v);

    friend void                   tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Level> const& o);
    friend std::unique_ptr<Level> tag_invoke(js::value_to_tag<std::unique_ptr<Level>>, js::value const& jv);

private:
    size_t nCol, nRow;
    bool   visible = true;

    std::vector<std::unique_ptr<TileLayer>> tileLayers;
    std::unique_ptr<EventLayer>             eventLayer;
    std::unique_ptr<SpecialTileLayer>       specialTileLayer;
};

#endif // LEVEL_H
