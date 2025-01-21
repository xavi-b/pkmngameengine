#ifndef SPECIALTILELAYER_H
#define SPECIALTILELAYER_H

#include "layer.h"

#include <memory>
#include <vector>

enum SpecialTileType
{
    GRASS = 1,
    WATER,
    TREE,
    ROCK,
    BOULDER,
    WATERFALL,
    LEDGE,
    SAND,
    TALLGRASS,
    UNDERWATERGRASS,
    SOOTGRASS,
    ICE,
    BREAKABLEICE,
    WATERCURRENT,
    BREAKABLEGROUND,
    __SIZE
};

static constexpr size_t SpecialTileTypeCount = SpecialTileType::__SIZE - SpecialTileType::GRASS;
std::string             SpecialTileTypeToString(SpecialTileType e);

using SpecialTileTypePtr = std::unique_ptr<SpecialTileType>;

void               tag_invoke(js::value_from_tag, js::value& jv, SpecialTileTypePtr const& o);
SpecialTileTypePtr tag_invoke(js::value_to_tag<SpecialTileTypePtr>, js::value const& jv);

SpecialTileTypePtr clone(SpecialTileTypePtr const& other);

class SpecialTileLayer : public Layer<SpecialTileTypePtr>
{
public:
    SpecialTileLayer(size_t nCol, size_t nRow);

    friend void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<SpecialTileLayer> const& o);
    friend std::unique_ptr<SpecialTileLayer> tag_invoke(js::value_to_tag<std::unique_ptr<SpecialTileLayer>>,
                                                        js::value const& jv);
};

#endif // SPECIALTILELAYER_H
