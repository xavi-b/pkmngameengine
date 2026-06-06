#include "encounterexperiencephase.h"

EncounterExperiencePhase::EncounterExperiencePhase(SingleBattleScene&                     scene,
                                                   std::shared_ptr<SingleBattleSceneData> data,
                                                   SDL_Renderer*                          renderer)
    : SingleBattleExperiencePhase(scene, data, renderer)
{
}

bool EncounterExperiencePhase::onExperienceResolvedNextPkmn()
{
    return false;
}
