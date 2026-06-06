#ifndef ENCOUNTERSCENE_H
#define ENCOUNTERSCENE_H

#include "singlebattlescene.h"

class EncounterScene : public SingleBattleScene
{
public:
    EncounterScene(SDL_Renderer* renderer);
    virtual ~EncounterScene();

    virtual std::string name() override;
    void                setEncounterPkmn(Pkmn::PkmnPtr const& newEncounterPkmn);

protected:
    virtual std::string encounterStartText() const override;

    virtual void initPhases() override;
};

#endif // ENCOUNTERSCENE_H
