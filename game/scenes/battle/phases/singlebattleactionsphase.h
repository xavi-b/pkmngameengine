#ifndef SINGLEBATTLEACTIONSPHASE_H
#define SINGLEBATTLEACTIONSPHASE_H

#include "singlebattlephase.h"

#include <memory>

class SingleBattleActionsPhase : public SingleBattlePhase
{
public:
    SingleBattleActionsPhase(SingleBattleScene&                     scene,
                             std::shared_ptr<SingleBattleSceneData> data,
                             SDL_Renderer*                          renderer);

    void init();
    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::unique_ptr<BattleActions> battleActions;
    std::unique_ptr<BattleSpeech>  battleSpeech;
};

#endif // SINGLEBATTLEACTIONSPHASE_H
