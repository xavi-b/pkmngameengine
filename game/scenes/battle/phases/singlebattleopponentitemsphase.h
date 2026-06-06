#ifndef SINGLEBATTLEOPPONENTITEMSPHASE_H
#define SINGLEBATTLEOPPONENTITEMSPHASE_H

#include "singlebattlephase.h"

class SingleBattleOpponentItemsPhase : public SingleBattlePhase
{
public:
    SingleBattleOpponentItemsPhase(SingleBattleScene&                     scene,
                                   std::shared_ptr<SingleBattleSceneData> data,
                                   SDL_Renderer*                          renderer);

    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;
};

#endif // SINGLEBATTLEOPPONENTITEMSPHASE_H
