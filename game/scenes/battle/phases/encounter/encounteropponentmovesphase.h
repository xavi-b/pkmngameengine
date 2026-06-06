#ifndef ENCOUNTEROPPONENTMOVESPHASE_H
#define ENCOUNTEROPPONENTMOVESPHASE_H

#include "../singlebattleopponentmovesphase.h"

class EncounterOpponentMovesPhase : public SingleBattleOpponentMovesPhase
{
public:
    EncounterOpponentMovesPhase(SingleBattleScene&                     scene,
                                std::shared_ptr<SingleBattleSceneData> data,
                                SDL_Renderer*                          renderer);
    std::string opponentMoveText(Move::MovePtr const& move) const override;
};

#endif // ENCOUNTEROPPONENTMOVESPHASE_H
