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

    friend void    tag_invoke(js::value_from_tag, js::value& jv, PkmnPtr const& o);
    friend PkmnPtr tag_invoke(js::value_to_tag<PkmnPtr>, js::value const& jv);

    Pkmn(PkmnDef::PkmnDefPtr definition, size_t level);

private:
    std::array<Move, 4> moves;
    PkmnDef::PkmnDefPtr definition;
    size_t              level = 0;
    std::string         nickName;
};

#endif // PKMN_H
