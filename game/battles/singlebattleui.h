#ifndef SINGLEBATTLEUI_H
#define SINGLEBATTLEUI_H

#include "fps.h"
#include "pkmn.h"

#include <SDL_image.h>

class SingleBattleUi
{
public:
    SingleBattleUi(SDL_Renderer* renderer);
    ~SingleBattleUi();

    void setFoePkmn(Pkmn::PkmnPtr const& newFoePkmn);
    void setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn);

    void draw(Fps const* fps, RenderSizes rs);

private:
    SDL_Renderer* renderer;
    SDL_Surface*  playerUiSurface;
    SDL_Texture*  playerUiTexture;
    SDL_Surface*  foeUiSurface;
    SDL_Texture*  foeUiTexture;
    SDL_Surface*  overlayHpSurface;
    SDL_Texture*  overlayHpTexture;
    SDL_Surface*  overlayXpSurface;
    SDL_Texture*  overlayXpTexture;

    Pkmn::PkmnPtr foePkmn;
    Pkmn::PkmnPtr playerPkmn;
};

#endif // SINGLEBATTLEUI_H
