#include "trainerplayeritemsphase.h"

TrainerPlayerItemsPhase::TrainerPlayerItemsPhase(SingleBattleScene&                     scene,
                                                 std::shared_ptr<SingleBattleSceneData> data,
                                                 SDL_Renderer*                          renderer)
    : SingleBattlePlayerItemsPhase(scene, data, renderer)
{
}

bool TrainerPlayerItemsPhase::canCaptureOpponent() const
{
    return false;
}
