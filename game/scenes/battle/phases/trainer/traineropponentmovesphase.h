#ifndef TRAINEROPPONENTMOVESPHASE_H
#define TRAINEROPPONENTMOVESPHASE_H

#include "../singlebattleopponentmovesphase.h"

class TrainerOpponentMovesPhase : public SingleBattleOpponentMovesPhase
{
public:
    TrainerOpponentMovesPhase(SingleBattleScene&                     scene,
                              std::shared_ptr<SingleBattleSceneData> data,
                              SDL_Renderer*                          renderer);
    std::string opponentMoveText(Move::MovePtr const& move) const override;
};

#endif // TRAINEROPPONENTMOVESPHASE_H
