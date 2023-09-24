#ifndef ENTITY_H
#define ENTITY_H

#include "sprite.h"

#include <memory>

class Entity
{
public:
    Entity();

private:
    std::shared_ptr<Sprite> sprite;
};

#endif // ENTITY_H
