#ifndef TITLEMENU_H
#define TITLEMENU_H

#include "scene.h"
#include <SDL_image.h>

class TitleMenu : public Scene
{
public:
    TitleMenu(SDL_Renderer* renderer);
    virtual ~TitleMenu();

    virtual void sendInputs(const Inputs* inputs) override;
    virtual void draw(const Fps* fps) override;

    virtual std::unique_ptr<Scene> nextScene() const override;

private:
    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;
};

#endif // TITLEMENU_H
