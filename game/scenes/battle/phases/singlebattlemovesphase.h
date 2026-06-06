#ifndef SINGLEBATTLEMOVESPHASE_H
#define SINGLEBATTLEMOVESPHASE_H

#include "singlebattlephase.h"

class SingleBattleMovesPhase : public SingleBattlePhase
{
public:
    SingleBattleMovesPhase(SingleBattleScene&                     scene,
                           std::shared_ptr<SingleBattleSceneData> data,
                           SDL_Renderer*                          renderer);

    void init();
    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::unique_ptr<TextSpeech>    noPpLeftSpeech;
    std::unique_ptr<MoveSelection> moveSelection;
};

#endif // SINGLEBATTLEMOVESPHASE_H
