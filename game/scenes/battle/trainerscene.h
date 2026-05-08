#ifndef TRAINERSCENE_H
#define TRAINERSCENE_H

#include "singlebattlescene.h"

class TrainerScene : public SingleBattleScene
{
public:
    TrainerScene(SDL_Renderer* renderer);
    virtual ~TrainerScene();

    virtual std::string name() override;
};

#endif // TRAINERSCENE_H
