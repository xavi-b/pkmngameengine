#ifndef TRAINERPLAYERRUNPHASE_H
#define TRAINERPLAYERRUNPHASE_H

#include "../singlebattleplayerrunphase.h"

class TrainerPlayerRunPhase : public SingleBattlePlayerRunPhase
{
public:
    TrainerPlayerRunPhase(SingleBattleScene&                     scene,
                          std::shared_ptr<SingleBattleSceneData> data,
                          SDL_Renderer*                          renderer);
    bool canPlayerRun() const override;
    bool tryPlayerRun() override;
};

#endif // TRAINERPLAYERRUNPHASE_H
