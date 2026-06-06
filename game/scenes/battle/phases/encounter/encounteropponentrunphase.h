#ifndef ENCOUNTEROPPONENTRUNPHASE_H
#define ENCOUNTEROPPONENTRUNPHASE_H

#include "../singlebattleopponentrunphase.h"

class EncounterOpponentRunPhase : public SingleBattleOpponentRunPhase
{
public:
    EncounterOpponentRunPhase(SingleBattleScene&                     scene,
                              std::shared_ptr<SingleBattleSceneData> data,
                              SDL_Renderer*                          renderer);
    std::string opponentRunText() const override;
};

#endif // ENCOUNTEROPPONENTRUNPHASE_H
