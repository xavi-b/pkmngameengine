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
    bool                     forcedSwitchUser   = false;
    size_t                   damageDealt        = 0;
    bool                     critical           = false;
    float                    effectiveness      = 1.f;
    bool                     hitPlayerSide      = false;
    bool                     statRose           = false;
    bool                     statRoseOnTarget   = false;
    bool                     healed             = false;
};

namespace MoveResolver
{
MoveResolveResult resolve(SingleBattleSceneData& data,
                          bool                   playerIsUser,
                          Move::MovePtr const&   move,
                          int                    callDepth         = 0,
                          bool                   instantTwoTurn    = false);
}

#endif // MOVERESOLVER_H
