#ifndef EVOLUTIONSCENE_H
#define EVOLUTIONSCENE_H

#include "pkmn.h"
#include "scene.h"
#include "textspeech.h"

class EvolutionScene : public Scene
{
public:
    EvolutionScene(SDL_Renderer* renderer, Pkmn::PkmnPtr const& pkmn, PkmnDef::PkmnDefPtr const& evolutionDef);
    virtual ~EvolutionScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::string name() override;

private:
    Pkmn::PkmnPtr               pkmn;
    PkmnDef::PkmnDefPtr         evolutionDef;
    bool                        leave = false;
    std::unique_ptr<TextSpeech> speech;
};

#endif // EVOLUTIONSCENE_H
