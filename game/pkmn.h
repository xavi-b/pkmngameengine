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
    using PkmnPtr = std::shared_ptr<Pkmn>;

    Pkmn(PkmnDef::PkmnDefPtr type, size_t level);

private:
    std::array<Move, 4> moves;
    PkmnDef::PkmnDefPtr type;
    size_t              level = 0;
};

#endif // PKMN_H
