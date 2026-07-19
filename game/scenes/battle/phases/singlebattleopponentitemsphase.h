#ifndef SINGLEBATTLEOPPONENTITEMSPHASE_H
#define SINGLEBATTLEOPPONENTITEMSPHASE_H

#include "singlebattlephase.h"
#include "textspeech.h"

class SingleBattleOpponentItemsPhase : public SingleBattlePhase
{
public:
    SingleBattleOpponentItemsPhase(SingleBattleScene&                     scene,
                                   std::shared_ptr<SingleBattleSceneData> data,
                                   SDL_Renderer*                          renderer);

    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::unique_ptr<TextSpeech> speech;
};

#endif // SINGLEBATTLEOPPONENTITEMSPHASE_H
