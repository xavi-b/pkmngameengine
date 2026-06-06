#ifndef TRAINERPLAYERITEMSPHASE_H
#define TRAINERPLAYERITEMSPHASE_H

#include "../singlebattleplayeritemsphase.h"

class TrainerPlayerItemsPhase : public SingleBattlePlayerItemsPhase
{
public:
    TrainerPlayerItemsPhase(SingleBattleScene&                     scene,
                            std::shared_ptr<SingleBattleSceneData> data,
                            SDL_Renderer*                          renderer);
    bool canCaptureOpponent() const override;
};

#endif // TRAINERPLAYERITEMSPHASE_H
