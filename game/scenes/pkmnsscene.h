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

    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;

    SDL_Surface* panelBlankSurface;
    SDL_Texture* panelBlankTexture;
    SDL_Surface* panelSurface;
    SDL_Texture* panelTexture;
    SDL_Surface* panelFaintSurface;
    SDL_Texture* panelFaintTexture;

    SDL_Surface* panelSelSurface;
    SDL_Texture* panelSelTexture;
    SDL_Surface* panelFaintSelSurface;
    SDL_Texture* panelFaintSelTexture;

    SDL_Surface* panelSwapSurface;
    SDL_Texture* panelSwapTexture;
    SDL_Surface* panelSwapSelSurface;
    SDL_Texture* panelSwapSelTexture;
    SDL_Surface* panelSwapSel2Surface;
    SDL_Texture* panelSwapSel2Texture;

    std::map<Pkmn::PkmnPtr, std::pair<SDL_Surface*, SDL_Texture*>> pkmnsRendering;

    size_t ticks = 0;

    bool           leave         = false;
    bool           selectingPkmn = false;
    Pkmn::PkmnPtr& selectedPkmn;
    size_t         currentIndex = 0;
    int            swapIndex    = -1;
};

#endif // PKMNSSCENE_H
