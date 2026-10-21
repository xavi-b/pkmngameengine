#include "encounterscenedata.h"

#include "statuscondition.h"
#include "utils.h"

namespace
{
bool latiasOrLatios(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn || !pkmn->getDefinition())
        return false;
    std::string id = pkmn->getDefinition()->getId();
    return id == "LATIAS" || id == "LATIOS";
}

bool tryWildFlee(SingleBattleSceneData const& data)
{
    if (data.opponentBattler.trapped || data.fairyLock)
        return false;

    size_t playerSpeed = battleSpeed(data.playerPkmn, &data.playerBattler, data.fieldWeather(), false);
    size_t wildSpeed   = battleSpeed(data.opponentPkmn, &data.opponentBattler, data.fieldWeather(), false);
    if (playerSpeed == 0)
        return true;
    size_t odds = wildSpeed * 128 / playerSpeed + 30;
    if (odds > 255)
        return true;
    return Utils::randuint(0, 255) < odds;
}
} // namespace

void EncounterSceneData::chooseOpponentAction()
{
    opponentAction = BattleActions::Type::MOVES;
    if (latiasOrLatios(opponentPkmn) && tryWildFlee(*this))
        opponentAction = BattleActions::Type::RUN;

    switch (opponentAction)
    {
    case BattleActions::RUN:
        state = OPPONENT_RUN;
        break;
    case BattleActions::BAG:
    case BattleActions::PKMNS:
        // Cannot happen for encounters
    case BattleActions::MOVES:
    default: {
        chooseOpponentMove();
        break;
    }
    }
}