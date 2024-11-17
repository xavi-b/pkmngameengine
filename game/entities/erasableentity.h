#ifndef ERASABLEENTITY_H
#define ERASABLEENTITY_H

#include "entity.h"

class ErasableEntity : public Entity
{
public:
    ErasableEntity();

    void   startErase();
    void   incrementEraseTicks();
    size_t eraseTicks();
    bool   erased() const;

private:
    bool   erase = false;
    size_t ticks = 0;
};

#endif // ERASABLEENTITY_H
