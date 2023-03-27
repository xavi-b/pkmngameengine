#ifndef TILE_H
#define TILE_H

#include <vector>
#include <memory>
#include "event.h"

class Tile
{
public:
    Tile();

    virtual void callEnterEvent(int param);
    virtual void callInterractionEvent(int param);

private:
    // TODO std::shared_ptr<Sprite> sprite;

    // TODO move events out
    std::unique_ptr<Event> enterEvent;
    std::unique_ptr<Event> interractionEvent;
};

#endif // TILE_H
