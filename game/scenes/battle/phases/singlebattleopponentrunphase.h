#ifndef SINGLEBATTLEOPPONENTRUNPHASE_H
#define SINGLEBATTLEOPPONENTRUNPHASE_H

#include "singlebattlephase.h"

class SingleBattleOpponentRunPhase : public SingleBattlePhase
{
public:
    SingleBattleOpponentRunPhase(SingleBattleScene&                     scene,
                                 std::shared_ptr<SingleBattleSceneData> data,
                                 SDL_Renderer*                          renderer);

    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string opponentRunText() const = 0;

private:
    std::unique_ptr<TextSpeech> runSpeech;
};

#endif // SINGLEBATTLEOPPONENTRUNPHASE_H
