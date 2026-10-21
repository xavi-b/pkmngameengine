#include "entryhazards.h"

#include "moveeffects.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

EntryHazardResult applyEntryHazards(SingleBattleSceneData& data, bool playerSide)
{
    EntryHazardResult result;
    auto              pkmn = playerSide ? data.playerPkmn : data.opponentPkmn;
    if (!pkmn || pkmn->isKO())
        return result;
    BattleBattler& battler = playerSide ? data.playerBattler : data.opponentBattler;

    int& spikes     = playerSide ? data.playerSpikes : data.opponentSpikes;
    bool magicGuard = !battler.abilitySuppressed && battler.abilityId == "MAGICGUARD";

    if (spikes > 0 && !magicGuard && grounded(pkmn, battler, data))
    {
        pkmn->decreaseHP(std::max<size_t>(1, pkmn->getTotalHP() / 8));
        result.messages.push_back(
            (boost::format(lc::translate("%1% is hurt by the spikes!")) % pkmn->getDisplayName()).str());
    }

    if (pkmn->isKO())
    {
        result.fainted = true;
        result.messages.push_back(
            (boost::format(lc::translate("%1% fainted !")) % pkmn->getDisplayName()).str());
    }
    return result;
}
