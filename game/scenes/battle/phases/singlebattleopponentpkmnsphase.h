#ifndef SINGLEBATTLEOPPONENTPKMNSPHASE_H
#define SINGLEBATTLEOPPONENTPKMNSPHASE_H

#include "singlebattlephase.h"

class SingleBattleOpponentPkmnsPhase : public SingleBattlePhase
{
public:
    SingleBattleOpponentPkmnsPhase(SingleBattleScene&                     scene,
                                   std::shared_ptr<SingleBattleSceneData> data,
                                   SDL_Renderer*                          renderer);

    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;
};

#endif // SINGLEBATTLEOPPONENTPKMNSPHASE_H
