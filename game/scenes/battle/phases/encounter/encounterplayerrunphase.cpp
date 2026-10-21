#include "encounterplayerrunphase.h"

#include "../../statuscondition.h"
#include "itemutils.h"
#include "utils.h"

EncounterPlayerRunPhase::EncounterPlayerRunPhase(SingleBattleScene&                     scene,
                                                 std::shared_ptr<SingleBattleSceneData> data,
                                                 SDL_Renderer*                          renderer)
    : SingleBattlePlayerRunPhase(scene, data, renderer)
{
}

bool EncounterPlayerRunPhase::canPlayerRun() const
{
    return true;
}

bool EncounterPlayerRunPhase::tryPlayerRun()
{
    bool runAway = !data->playerBattler.abilitySuppressed && data->playerBattler.abilityId == "RUNAWAY";
    bool smoke   = ItemUtils::activeHeldId(data->playerPkmn, &data->playerBattler) == "SMOKEBALL";
    if (runAway || smoke)
        return true;

    ++data->runAttemps;
    size_t playerSpeed = battleSpeed(data->playerPkmn, &data->playerBattler, data->fieldWeather(), false);
    size_t wildSpeed   = battleSpeed(data->opponentPkmn, &data->opponentBattler, data->fieldWeather(), false);
    if (wildSpeed == 0)
        return true;
    size_t odds = playerSpeed * 128 / wildSpeed + 30 * data->runAttemps;
    if (odds > 255)
        return true;
    return Utils::randuint(0, 255) < odds;
}