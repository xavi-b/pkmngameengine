#ifndef SINGLEBATTLEPLAYERPKMNSPHASE_H
#define SINGLEBATTLEPLAYERPKMNSPHASE_H

#include "singlebattlephase.h"

#include <string>

class SingleBattlePlayerPkmnsPhase : public SingleBattlePhase
{
public:
    SingleBattlePlayerPkmnsPhase(SingleBattleScene&                     scene,
                                 std::shared_ptr<SingleBattleSceneData> data,
                                 SDL_Renderer*                          renderer);

    void showEnterSpeech(std::string const& text);
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::unique_ptr<TextSpeech> pkmnEnterSpeech;
    std::unique_ptr<TextSpeech> hazardSpeech;
    bool                        sendOutStarted      = false;
    bool                        hazardsDone         = false;
    bool                        hazardFainted       = false;
    bool                        pursuitResolved     = false;
    bool                        pursuitConsumedTurn = false;
    bool                        trapChecked         = false;
};

#endif // SINGLEBATTLEPLAYERPKMNSPHASE_H
