#ifndef PKMN_H
#define PKMN_H

#include "pkmndef.h"

#include <array>
#include <vector>

struct Move
{
};

class Pkmn
{
public:
    Pkmn();

private:
    std::array<Move, 4> moves;
    PkmnDef::PkmnDefPtr type;
};

#endif // PKMN_H
