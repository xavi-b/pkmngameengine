#ifndef ENCOUNTERPLAYERMOVESPHASE_H
#define ENCOUNTERPLAYERMOVESPHASE_H

#include "../singlebattleplayermovesphase.h"

class EncounterPlayerMovesPhase : public SingleBattlePlayerMovesPhase
{
public:
    EncounterPlayerMovesPhase(SingleBattleScene&                     scene,
                              std::shared_ptr<SingleBattleSceneData> data,
                              SDL_Renderer*                          renderer);
    float battleExperienceMultiplier() const override;
    void  onOpponentPkmnDefeated() override;
};

#endif // ENCOUNTERPLAYERMOVESPHASE_H
