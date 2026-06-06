#include "singlebattleopponentitemsphase.h"

#include "../singlebattlescene.h"

SingleBattleOpponentItemsPhase::SingleBattleOpponentItemsPhase(SingleBattleScene&                     scene,
                                                               std::shared_ptr<SingleBattleSceneData> data,
                                                               SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleOpponentItemsPhase::update(Inputs const* /*inputs*/)
{
    // TODO: Attached berry

    data->state = WEATHER;
}

void SingleBattleOpponentItemsPhase::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}
