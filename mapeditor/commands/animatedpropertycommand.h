#ifndef ANIMATEDPROPERTYCOMMAND_H
#define ANIMATEDPROPERTYCOMMAND_H

#include "mappercommand.h"
#include "tile.h"

class AnimatedPropertyCommand : public MapperCommand
{
public:
    AnimatedPropertyCommand(Tile::TilePtr& tile)
        : MapperCommand(), tile(tile)
    {
    }

    virtual void execute() override
    {
        tile->setAnimated(!tile->isAnimated());
    }

    virtual void undo() override
    {
        execute();
    }

protected:
    Tile::TilePtr& tile;
};

#endif // ANIMATEDPROPERTYCOMMAND_H
