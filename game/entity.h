#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include "sprite.h"

class Entity
{
public:
    Entity();

private:
    std::shared_ptr<Sprite> sprite;
};

#endif // ENTITY_H
