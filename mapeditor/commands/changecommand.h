#ifndef CHANGECOMMAND_H
#define CHANGECOMMAND_H

#include "mappercommand.h"

#include <memory>

template <class T>
class ChangeCommand : public MapperCommand
{
public:
    ChangeCommand(std::unique_ptr<T>& previousTile, std::unique_ptr<T> newTile)
        : MapperCommand(), previousTile(previousTile), newTile(std::move(newTile))
    {
    }

    virtual void execute() override
    {
        previousTile.swap(newTile);
    }

    virtual void undo() override
    {
        execute();
    }

protected:
    std::unique_ptr<T>& previousTile;
    std::unique_ptr<T>  newTile;
};

#endif // CHANGECOMMAND_H
