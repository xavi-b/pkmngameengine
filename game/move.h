#ifndef MOVE_H
#define MOVE_H

#include "movedef.h"

#include <array>
#include <vector>

class Move
{
public:
    using MovePtr = std::shared_ptr<Move>;

    friend void    tag_invoke(js::value_from_tag, js::value& jv, MovePtr const& o);
    friend MovePtr tag_invoke(js::value_to_tag<MovePtr>, js::value const& jv);

    Move(MoveDef::MoveDefPtr definition);

private:
    MoveDef::MoveDefPtr definition;
    size_t              currentPP = 0;
};

#endif // MOVE_H
