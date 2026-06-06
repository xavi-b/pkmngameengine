#ifndef TRAINEROPPONENTRUNPHASE_H
#define TRAINEROPPONENTRUNPHASE_H

#include "../singlebattleopponentrunphase.h"

class TrainerOpponentRunPhase : public SingleBattleOpponentRunPhase
{
public:
    TrainerOpponentRunPhase(SingleBattleScene&                     scene,
                            std::shared_ptr<SingleBattleSceneData> data,
                            SDL_Renderer*                          renderer);
    std::string opponentRunText() const override;
};

#endif // TRAINEROPPONENTRUNPHASE_H
