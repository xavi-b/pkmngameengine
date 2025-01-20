#ifndef ENTITY_H
#define ENTITY_H

#include <string>

class Entity
{
public:
    enum class Direction
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
    virtual ~Entity();

    Direction direction         = Direction::NONE;
    Direction previousDirection = Direction::NONE;
    size_t    l                 = 0;
    size_t    x                 = 0;
    size_t    y                 = 0;
    size_t    previousX         = 0;
    size_t    previousY         = 0;
    Speed     speed             = Speed::WALK;
    Speed     previousSpeed     = Speed::WALK;
    bool      surfing           = false;
    bool      boulder           = false;

    static Direction   getOppositeDirection(Direction direction);
    static std::string getDirectionString(Direction direction);
    static Direction   getDirectionFromString(std::string direction);
};

#endif // ENTITY_H
