#ifndef TILE_H
#define TILE_H

#include <vector>
#include <memory>
#include <string>
#include "event.h"

class Tile
{
public:
    Tile(const std::string& spritePath, int row, int col);

    const std::string& getSpritePath() const;
    int                getRow() const;
    int                getCol() const;

    virtual void callEnterEvent(int param);
    virtual void callInterractionEvent(int param);

private:
    std::string spritePath;
    int         row = 0;
    int         col = 0;

    // TODO move events out
    std::unique_ptr<Event> enterEvent;
    std::unique_ptr<Event> interractionEvent;
};

#endif // TILE_H
