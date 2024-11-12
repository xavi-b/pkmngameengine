#ifndef CHANGESCOMMAND_H
#define CHANGESCOMMAND_H

#include "mappercommand.h"

#include <memory>
#include <vector>

template <class T>
class ChangesCommand : public MapperCommand
{
public:
    ChangesCommand(std::vector<std::unique_ptr<T>*> const& previousTiles, std::vector<std::unique_ptr<T>>&& newTiles)
        : MapperCommand(), previousTiles(previousTiles), newTiles(std::move(newTiles))
    {
    }

    virtual void execute() override
    {
        for (size_t i = 0; i < previousTiles.size(); ++i)
            previousTiles[i]->swap(newTiles[i]);
    }

    virtual void undo() override
    {
        execute();
    }

protected:
    std::vector<std::unique_ptr<T>*> previousTiles;
    std::vector<std::unique_ptr<T>>  newTiles;
};

#endif // CHANGESCOMMAND_H
