#ifndef TILELAYER_H
#define TILELAYER_H

#include <vector>
#include <memory>
#include <string>
#include "tile.h"
#include "layer.h"

class TileLayer : public Layer<Tile::TilePtr>
{
public:
    enum Type
    {
        GROUND,
        SOLID,
        OVERLAY
    };

    TileLayer(Type type, int nCol, int nRow);

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
