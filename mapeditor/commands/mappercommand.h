#ifndef MAPPERCOMMAND_H
#define MAPPERCOMMAND_H

class MapperCommand
{
public:
    MapperCommand();

    virtual void execute() = 0;
    virtual void undo()    = 0;
};

#endif // MAPPERCOMMAND_H
