#include "entity.h"

Entity::Entity()
{
}

Entity::~Entity()
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

std::string Entity::getDirectionString(Direction direction)
{
    switch (direction)
    {
    case Direction::LEFT:
        return "LEFT";
    case Direction::RIGHT:
        return "RIGHT";
    case Direction::UP:
        return "UP";
    case Direction::DOWN:
    case Direction::NONE:
    default:
        return "DOWN";
    }
}

Entity::Direction Entity::getDirectionFromString(std::string direction)
{
    if (direction == "LEFT")
        return Direction::LEFT;
    if (direction == "RIGHT")
        return Direction::RIGHT;
    if (direction == "UP")
        return Direction::UP;
    if (direction == "DOWN")
        return Direction::DOWN;

    return Direction::NONE;
}
