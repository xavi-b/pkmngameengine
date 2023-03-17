#ifndef TILE_H
#define TILE_H

#include <vector>
#include <memory>
#include "event.h"
#include "sprite.h"

class Tile
{
public:
    Tile();

    virtual void callEnterEvent(int param);
    virtual void callInterractionEvent(int param);

private:
    std::unique_ptr<Event> enterEvent;
    std::unique_ptr<Event> interractionEvent;
    std::shared_ptr<Sprite> sprite;

    bool solid = false;
};

#endif // TILE_H
