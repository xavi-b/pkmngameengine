#include "trainerplayerrunphase.h"

TrainerPlayerRunPhase::TrainerPlayerRunPhase(SingleBattleScene&                     scene,
                                             std::shared_ptr<SingleBattleSceneData> data,
                                             SDL_Renderer*                          renderer)
    : SingleBattlePlayerRunPhase(scene, data, renderer)
{
}

bool TrainerPlayerRunPhase::canPlayerRun() const
{
    return false;
}

bool TrainerPlayerRunPhase::tryPlayerRun()
{
    return false;
}