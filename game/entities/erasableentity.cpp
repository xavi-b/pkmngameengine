#include "erasableentity.h"

ErasableEntity::ErasableEntity()
{
}

void ErasableEntity::startErase()
{
    erase = true;
}

void ErasableEntity::incrementEraseTicks()
{
    if (erase)
        ticks++;
}

size_t ErasableEntity::eraseTicks()
{
    return ticks;
}

bool ErasableEntity::erased() const
{
    return ticks >= 4;
}
