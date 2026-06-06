#include "encounterplayerrunphase.h"

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
    ++data->runAttemps;
    bool   run       = true;
    size_t wildSpeed = data->opponentPkmn ? data->opponentPkmn->getStats()[PkmnDef::SPEED] : 0;

    if (wildSpeed > 0)
    {
        // https://bulbapedia.bulbagarden.net/wiki/Escape#Generation_III_and_IV
        size_t playerSpeed = data->playerPkmn ? data->playerPkmn->getStats()[PkmnDef::SPEED] : 0;
        size_t odds        = ((playerSpeed * 128 / wildSpeed) + 30 * data->runAttemps) % 256;
        size_t random      = Utils::randuint(0, 255);
        run                = random < odds;
    }

    return run;
}