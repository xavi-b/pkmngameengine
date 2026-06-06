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
};

#endif // SINGLEBATTLEPLAYERPKMNSPHASE_H
