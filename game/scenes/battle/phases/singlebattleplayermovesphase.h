#ifndef SINGLEBATTLEPLAYERMOVESPHASE_H
#define SINGLEBATTLEPLAYERMOVESPHASE_H

#include "singlebattlephase.h"

class SingleBattlePlayerMovesPhase : public SingleBattlePhase
{
public:
    SingleBattlePlayerMovesPhase(SingleBattleScene&                     scene,
                                 std::shared_ptr<SingleBattleSceneData> data,
                                 SDL_Renderer*                          renderer);

    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

    virtual float battleExperienceMultiplier() const = 0;
    virtual void  onOpponentPkmnDefeated()           = 0;

private:
    std::unique_ptr<TextSpeech> playerMoveSpeech;
};

#endif // SINGLEBATTLEPLAYERMOVESPHASE_H
