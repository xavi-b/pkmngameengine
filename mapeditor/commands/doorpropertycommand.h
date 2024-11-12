#ifndef DOORPROPERTYCOMMAND_H
#define DOORPROPERTYCOMMAND_H

#include "mappercommand.h"
#include "tile.h"

class DoorPropertyCommand : public MapperCommand
{
public:
    DoorPropertyCommand(Tile::TilePtr& tile)
        : MapperCommand(), tile(tile)
    {
    }

    virtual void execute() override
    {
        tile->setDoor(!tile->isDoor());
    }

    virtual void undo() override
    {
        execute();
    }

protected:
    Tile::TilePtr& tile;
};

#endif // DOORPROPERTYCOMMAND_H
