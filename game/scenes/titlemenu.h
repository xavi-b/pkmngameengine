#ifndef TITLEMENU_H
#define TITLEMENU_H

#include "pkmn.h"
#include "scene.h"
#include "sprite.h"

#include <SDL_image.h>
#include <map>

class TitleMenu : public Scene
{
public:
    TitleMenu(SDL_Renderer* renderer);
    virtual ~TitleMenu();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    enum Index : char
    {
        Continue = 0,
        NewGame,
        Options
    };

    SDL_Surface*                                                   bgSurface;
    SDL_Texture*                                                   bgTexture;
    SDL_Surface*                                                   panelSurface;
    SDL_Texture*                                                   panelTexture;
    SDL_Surface*                                                   playerSurface;
    SDL_Texture*                                                   playerTexture;
    std::map<Pkmn::PkmnPtr, std::pair<SDL_Surface*, SDL_Texture*>> pkmnsRendering;

    size_t ticks = 0;

    bool  goToGame       = false;
    bool  goToIntroScene = false;
    bool  goToTitleScene = false;
    Index currentIndex   = Continue;
};

#endif // TITLEMENU_H
