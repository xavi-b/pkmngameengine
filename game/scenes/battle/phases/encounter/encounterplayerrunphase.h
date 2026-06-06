#ifndef ENCOUNTERPLAYERRUNPHASE_H
#define ENCOUNTERPLAYERRUNPHASE_H

#include "../singlebattleplayerrunphase.h"

class EncounterPlayerRunPhase : public SingleBattlePlayerRunPhase
{
public:
    EncounterPlayerRunPhase(SingleBattleScene&                     scene,
                            std::shared_ptr<SingleBattleSceneData> data,
                            SDL_Renderer*                          renderer);
    bool canPlayerRun() const override;
    bool tryPlayerRun() override;
};

#endif // ENCOUNTERPLAYERRUNPHASE_H
