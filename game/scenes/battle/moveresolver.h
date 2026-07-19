#ifndef MOVERESOLVER_H
#define MOVERESOLVER_H

#include "movedef.h"
#include "move.h"
#include "pkmn.h"
#include "singlebattlescenedata.h"

#include <string>
#include <vector>

struct MoveResolveResult
{
    std::vector<std::string> messages;
    bool                     hit                = false;
    bool                     targetFainted      = false;
    bool                     userFainted        = false;
    bool                     shouldSwitchUser   = false;
    bool                     shouldSwitchTarget = false;
    bool                     forcedSwitchTarget = false;
};

namespace MoveResolver
{
MoveResolveResult resolve(SingleBattleSceneData& data,
                          bool                   playerIsUser,
                          Move::MovePtr const&   move);
}

#endif // MOVERESOLVER_H
