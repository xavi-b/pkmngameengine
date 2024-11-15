#include "entity.h"

Entity::Entity()
{
}

Entity::Direction Entity::getOppositeDirection(Direction direction)
{
    switch (direction)
    {
    case Direction::LEFT:
        return Direction::RIGHT;
    case Direction::RIGHT:
        return Direction::LEFT;
    case Direction::UP:
        return Direction::DOWN;
    case Direction::DOWN:
    case Direction::NONE:
    default:
        return Direction::UP;
    }
}
