#ifndef TILELAYER_H
#define TILELAYER_H

#include "layer.h"
#include "tile.h"

#include <memory>
#include <string>
#include <array>

class TileLayer : public Layer<Tile::TilePtr>
{
public:
    static constexpr std::array<unsigned char, 4> NightColor = {14, 13, 91, 175};

    enum Type
    {
        GROUND,
        GROUND_OVERLAY,
        GROUND_LIGHTS,
        SOLID,
        SOLID_OVERLAY,
        SOLID_LIGHTS,
        OVERLAY,
        OVERLAY_LIGHTS,
        __SIZE
    };

    TileLayer(Type type, size_t nCol, size_t nRow);

    Type        getType() const;
    std::string getTypeName() const;
    bool        isVisible() const;
    void        setVisible(bool value);

    friend void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<TileLayer> const& o);
    friend std::unique_ptr<TileLayer> tag_invoke(js::value_to_tag<std::unique_ptr<TileLayer>>, js::value const& jv);

protected:
    Type type;

    bool visible = true;
};

#endif // LAYER_H
