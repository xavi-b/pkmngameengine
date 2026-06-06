#include "traineropponentrunphase.h"

#include "utils.h"

TrainerOpponentRunPhase::TrainerOpponentRunPhase(SingleBattleScene&                     scene,
                                                 std::shared_ptr<SingleBattleSceneData> data,
                                                 SDL_Renderer*                          renderer)
    : SingleBattleOpponentRunPhase(scene, data, renderer)
{
}

std::string TrainerOpponentRunPhase::opponentRunText() const
{
    return lc::translate("The trainer withdrew !");
}
