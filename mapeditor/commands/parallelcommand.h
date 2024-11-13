#ifndef PARALLELCOMMAND_H
#define PARALLELCOMMAND_H

#include "mappercommand.h"

#include <memory>
#include <vector>

class ParallelCommand : public MapperCommand
{
public:
    ParallelCommand();

    void addCommand(std::unique_ptr<MapperCommand> cmd);

    virtual void execute();
    virtual void undo();

private:
    std::vector<std::unique_ptr<MapperCommand>> commands;
};

#endif // PARALLELCOMMAND_H
