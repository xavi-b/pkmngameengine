#include "encounterplayeritemsphase.h"

EncounterPlayerItemsPhase::EncounterPlayerItemsPhase(SingleBattleScene&                     scene,
                                                     std::shared_ptr<SingleBattleSceneData> data,
                                                     SDL_Renderer*                          renderer)
    : SingleBattlePlayerItemsPhase(scene, data, renderer)
{
}

bool EncounterPlayerItemsPhase::canCaptureOpponent() const
{
    return true;
}
