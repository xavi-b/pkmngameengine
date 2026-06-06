#ifndef SINGLEBATTLEPLAYERITEMSPHASE_H
#define SINGLEBATTLEPLAYERITEMSPHASE_H

#include "singlebattlephase.h"

class SingleBattlePlayerItemsPhase : public SingleBattlePhase
{
public:
    SingleBattlePlayerItemsPhase(SingleBattleScene&                     scene,
                                 std::shared_ptr<SingleBattleSceneData> data,
                                 SDL_Renderer*                          renderer);

    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool canCaptureOpponent() const = 0;

private:
    std::unique_ptr<TextSpeech> itemUseSpeech;
};

#endif // SINGLEBATTLEPLAYERITEMSPHASE_H
