#ifndef ENTRYHAZARDS_H
#define ENTRYHAZARDS_H

#include "singlebattlescenedata.h"

#include <string>
#include <vector>

struct EntryHazardResult
{
    std::vector<std::string> messages;
    bool                     fainted = false;
};

// Spikes, toxic spikes, stealth rock, and sticky web for the side that just switched in.
EntryHazardResult applyEntryHazards(SingleBattleSceneData& data, bool playerSide);

#endif // ENTRYHAZARDS_H
