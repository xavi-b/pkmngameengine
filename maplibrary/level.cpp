#include "level.h"

Level::Level(size_t nCol, size_t nRow) : nCol(nCol), nRow(nRow)
{
    addTileLayer(TileLayer::Type::GROUND);
    addTileLayer(TileLayer::Type::GROUND_OVERLAY);
    addTileLayer(TileLayer::Type::SOLID);
    addTileLayer(TileLayer::Type::SOLID_OVERLAY);
    addTileLayer(TileLayer::Type::OVERLAY);
    eventLayer       = std::make_unique<EventLayer>(nCol, nRow);
    specialTileLayer = std::make_unique<SpecialTileLayer>(nCol, nRow);
}

void Level::addTileLayer(TileLayer::Type type)
{
    tileLayers.emplace_back(std::make_unique<TileLayer>(type, nCol, nRow));
}

std::vector<std::unique_ptr<TileLayer>>& Level::getTileLayers()
{
    return tileLayers;
}

std::unique_ptr<EventLayer>& Level::getEventLayer()
{
    return eventLayer;
}

std::unique_ptr<SpecialTileLayer>& Level::getSpecialTileLayer()
{
    return specialTileLayer;
}

bool Level::isVisible() const
{
    return visible;
}

void Level::setVisible(bool value)
{
    visible = value;
}

void Level::setNCol(size_t v)
{
    nCol = v;
    for (size_t i = 0; i < tileLayers.size(); ++i)
        tileLayers[i]->setNCol(v);
    eventLayer->setNCol(v);
    specialTileLayer->setNCol(v);
}

void Level::setNRow(size_t v)
{

    nRow = v;
    for (size_t i = 0; i < tileLayers.size(); ++i)
        tileLayers[i]->setNRow(v);
    eventLayer->setNRow(v);
    specialTileLayer->setNRow(v);
}

void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Level> const& o)
{
    jv = {
        {"nCol",             o->nCol                                                                      },
        {"nRow",             o->nRow                                                                      },
        {"visible",          o->visible                                                                   },
        {"tileLayers",       js::value_from<std::vector<std::unique_ptr<TileLayer>> const&>(o->tileLayers)},
        {"eventLayer",       js::value_from<std::unique_ptr<EventLayer> const&>(o->eventLayer)            },
        {"specialTileLayer", js::value_from<std::unique_ptr<SpecialTileLayer> const&>(o->specialTileLayer)}
    };
}

std::unique_ptr<Level> tag_invoke(js::value_to_tag<std::unique_ptr<Level>>, js::value const& jv)
{
    js::object const&      obj = jv.as_object();
    std::unique_ptr<Level> o =
        std::make_unique<Level>(js::value_to<size_t>(obj.at("nCol")), js::value_to<size_t>(obj.at("nRow")));

    o->tileLayers       = js::value_to<std::vector<std::unique_ptr<TileLayer>>>(obj.at("tileLayers"));
    o->eventLayer       = js::value_to<std::unique_ptr<EventLayer>>(obj.at("eventLayer"));
    o->specialTileLayer = js::value_to<std::unique_ptr<SpecialTileLayer>>(obj.at("specialTileLayer"));

    return o;
}
