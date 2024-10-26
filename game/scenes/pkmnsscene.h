#ifndef PKMNSSCENE_H
#define PKMNSSCENE_H

#include "pkmn.h"
#include "scene.h"

#include <SDL_image.h>

class PkmnsScene : public Scene
{
public:
    PkmnsScene(SDL_Renderer* renderer, Pkmn::PkmnPtr& currentPkmn);
    virtual ~PkmnsScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::string name() override;

private:
    void selectPkmn(Pkmn::PkmnPtr newSelectedPkmn);

    bool           leave = false;
    Pkmn::PkmnPtr& selectedPkmn;
};

#endif // PKMNSSCENE_H
