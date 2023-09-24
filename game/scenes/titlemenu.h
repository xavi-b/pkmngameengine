#ifndef TITLEMENU_H
#define TITLEMENU_H

#include "scene.h"

#include <SDL_image.h>

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
        Options,
        Quit
    };

    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;
    SDL_Surface* panelSurface;
    SDL_Texture* panelTexture;

    bool  goToGame       = false;
    bool  goToIntroScene = false;
    bool  goToTitleScene = false;
    Index currentIndex   = Continue;
};

#endif // TITLEMENU_H
