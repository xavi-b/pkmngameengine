#ifndef SINGLEBATTLEOPPONENTPKMNSPHASE_H
#define SINGLEBATTLEOPPONENTPKMNSPHASE_H

#include "singlebattlephase.h"
#include "textspeech.h"

class SingleBattleOpponentPkmnsPhase : public SingleBattlePhase
{
public:
    SingleBattleOpponentPkmnsPhase(SingleBattleScene&                     scene,
                                   std::shared_ptr<SingleBattleSceneData> data,
                                   SDL_Renderer*                          renderer);

    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::unique_ptr<TextSpeech> speech;
    bool                        throwStarted  = false;
    bool                        hazardsDone   = false;
    bool                        hazardFainted = false;
    bool                        pursuitDone   = false;
    bool                        trapChecked   = false;
    bool                        trapBlocked   = false;
};

#endif // SINGLEBATTLEOPPONENTPKMNSPHASE_H
