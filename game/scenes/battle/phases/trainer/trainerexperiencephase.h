#ifndef TRAINEREXPERIENCEPHASE_H
#define TRAINEREXPERIENCEPHASE_H

#include "../singlebattleexperiencephase.h"

class TrainerExperiencePhase : public SingleBattleExperiencePhase
{
public:
    TrainerExperiencePhase(SingleBattleScene&                     scene,
                           std::shared_ptr<SingleBattleSceneData> data,
                           SDL_Renderer*                          renderer);
    bool onExperienceResolvedNextPkmn() override;

protected:
    std::vector<std::string> victoryTexts() override;
};

#endif // TRAINEREXPERIENCEPHASE_H
