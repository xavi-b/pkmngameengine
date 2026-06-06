#ifndef ENCOUNTERPLAYERITEMSPHASE_H
#define ENCOUNTERPLAYERITEMSPHASE_H

#include "../singlebattleplayeritemsphase.h"

class EncounterPlayerItemsPhase : public SingleBattlePlayerItemsPhase
{
public:
    EncounterPlayerItemsPhase(SingleBattleScene&                     scene,
                              std::shared_ptr<SingleBattleSceneData> data,
                              SDL_Renderer*                          renderer);
    bool canCaptureOpponent() const override;
};

#endif // ENCOUNTERPLAYERITEMSPHASE_H
