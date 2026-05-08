#ifndef ENCOUNTERSCENE_H
#define ENCOUNTERSCENE_H

#include "singlebattlescene.h"

class EncounterScene : public SingleBattleScene
{
public:
    EncounterScene(SDL_Renderer* renderer);
    virtual ~EncounterScene();

    virtual std::string name() override;
};

#endif // ENCOUNTERSCENE_H
