#include "encounterplayermovesphase.h"

EncounterPlayerMovesPhase::EncounterPlayerMovesPhase(SingleBattleScene&                     scene,
                                                     std::shared_ptr<SingleBattleSceneData> data,
                                                     SDL_Renderer*                          renderer)
    : SingleBattlePlayerMovesPhase(scene, data, renderer)
{
}

float EncounterPlayerMovesPhase::battleExperienceMultiplier() const
{
    return 1.0f;
}

void EncounterPlayerMovesPhase::onOpponentPkmnDefeated()
{
}
