#ifndef ENCOUNTERSCENE_H
#define ENCOUNTERSCENE_H

#include "battlescene.h"
#include "pkmn.h"

#include <SDL_image.h>

class EncounterScene : public BattleScene
{
public:
    EncounterScene(SDL_Renderer* renderer);
    virtual ~EncounterScene();

    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string name() override;

    void setEncounteredPkmn(Pkmn::PkmnPtr const& newEncounteredPkmn);

private:
    Pkmn::PkmnPtr encounteredPkmn;
};

#endif // ENCOUNTERSCENE_H
