#ifndef RESETCOMMAND_H
#define RESETCOMMAND_H

#include "changecommand.h"

#include <memory>

template <class T>
class ResetCommand : public ChangeCommand<T>
{
public:
    ResetCommand(std::unique_ptr<T>& previousTile) : ChangeCommand<T>(previousTile, nullptr)
    {
    }
};

#endif // RESETCOMMAND_H
