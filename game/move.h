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

    size_t getCurrentPP() const;
    void   setCurrentPP(size_t newCurrentPP);
    void   decreasePP();

    // 0–3 PP Ups. Max PP is base + base * 20 * ups / 100.
    size_t getPpUps() const;
    void   setPpUps(size_t newPpUps);
    size_t maxPP() const;

    MoveDef::MoveDefPtr getDefinition() const;

private:
    MoveDef::MoveDefPtr definition;
    size_t              currentPP = 0;
    size_t              ppUps     = 0;
};

#endif // MOVE_H
