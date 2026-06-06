#ifndef TRAINERPLAYERMOVESPHASE_H
#define TRAINERPLAYERMOVESPHASE_H

#include "../singlebattleplayermovesphase.h"

class TrainerPlayerMovesPhase : public SingleBattlePlayerMovesPhase
{
public:
    TrainerPlayerMovesPhase(SingleBattleScene&                     scene,
                            std::shared_ptr<SingleBattleSceneData> data,
                            SDL_Renderer*                          renderer);
    float battleExperienceMultiplier() const override;
    void  onOpponentPkmnDefeated() override;
};

#endif // TRAINERPLAYERMOVESPHASE_H
