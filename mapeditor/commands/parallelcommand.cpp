#include "parallelcommand.h"

ParallelCommand::ParallelCommand() : MapperCommand()
{
}

void ParallelCommand::addCommand(std::unique_ptr<MapperCommand> cmd)
{
    commands.push_back(std::move(cmd));
}

void ParallelCommand::execute()
{
    for (auto it = commands.begin(); it != commands.end(); ++it)
        it->get()->execute();
}

void ParallelCommand::undo()
{
    for (auto it = commands.begin(); it != commands.end(); ++it)
        it->get()->undo();
}
