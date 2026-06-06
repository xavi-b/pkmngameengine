#include "singlebattleopponentpkmnsphase.h"

#include "../singlebattlescene.h"

SingleBattleOpponentPkmnsPhase::SingleBattleOpponentPkmnsPhase(SingleBattleScene&                     scene,
                                                               std::shared_ptr<SingleBattleSceneData> data,
                                                               SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleOpponentPkmnsPhase::update(Inputs const* /*inputs*/)
{
    // Should never reach
}

void SingleBattleOpponentPkmnsPhase::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    // Should never reach
}
