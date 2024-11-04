#ifndef ENTITY_H
#define ENTITY_H

#include <memory>

class Entity
{
public:
    enum Direction
    {
        NONE,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    enum Speed
    {
        BIKE = 1,
        RUN  = 2,
        WALK = 3
    };

    Entity();

    Direction direction     = NONE;
    size_t    l             = 0;
    int       x             = 0;
    int       y             = 0;
    int       previousX     = 0;
    int       previousY     = 0;
    Speed     speed         = WALK;
    Speed     previousSpeed = WALK;
};

#endif // ENTITY_H
