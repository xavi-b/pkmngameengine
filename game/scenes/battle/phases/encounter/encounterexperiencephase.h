#ifndef ENCOUNTEREXPERIENCEPHASE_H
#define ENCOUNTEREXPERIENCEPHASE_H

#include "../singlebattleexperiencephase.h"

class EncounterExperiencePhase : public SingleBattleExperiencePhase
{
public:
    EncounterExperiencePhase(SingleBattleScene&                     scene,
                             std::shared_ptr<SingleBattleSceneData> data,
                             SDL_Renderer*                          renderer);
    bool onExperienceResolvedNextPkmn() override;
};

#endif // ENCOUNTEREXPERIENCEPHASE_H
