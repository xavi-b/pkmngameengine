#include "level.h"

Level::Level(size_t nCol, size_t nRow) : nCol(nCol), nRow(nRow)
{
    addTileLayer(TileLayer::Type::GROUND);
    addTileLayer(TileLayer::Type::SOLID);
    addTileLayer(TileLayer::Type::OVERLAY);
    eventLayer      = std::make_unique<EventLayer>(nCol, nRow);
    encountersLayer = std::make_unique<EncountersLayer>(nCol, nRow);
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
    encountersLayer->setNCol(v);
}

void Level::setNRow(size_t v)
{

    nRow = v;
    for (size_t i = 0; i < tileLayers.size(); ++i)
        tileLayers[i]->setNRow(v);
    eventLayer->setNRow(v);
    encountersLayer->setNRow(v);
}

void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<Level> const& o)
{
    jv = {{"nCol", o->nCol},
          {"nRow", o->nRow},
          {"visible", o->visible},
          {"tileLayers", js::value_from<std::vector<std::unique_ptr<TileLayer>> const&>(o->tileLayers)},
          {"eventLayer", js::value_from<std::unique_ptr<EventLayer> const&>(o->eventLayer)},
          {"encountersLayer", js::value_from<std::unique_ptr<EncountersLayer> const&>(o->encountersLayer)}};
}

std::unique_ptr<Level> tag_invoke(js::value_to_tag<std::unique_ptr<Level>>, js::value const& jv)
{
    js::object const&      obj = jv.as_object();
    std::unique_ptr<Level> o =
        std::make_unique<Level>(js::value_to<size_t>(obj.at("nCol")), js::value_to<size_t>(obj.at("nRow")));

    o->tileLayers      = js::value_to<std::vector<std::unique_ptr<TileLayer>>>(obj.at("tileLayers"));
    o->eventLayer      = js::value_to<std::unique_ptr<EventLayer>>(obj.at("eventLayer"));
    o->encountersLayer = js::value_to<std::unique_ptr<EncountersLayer>>(obj.at("encountersLayer"));

    return o;
}
