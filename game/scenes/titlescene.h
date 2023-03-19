#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "scene.h"
#include <SDL_image.h>
#include <SDL_mixer.h>

class TitleScene : public Scene
{
public:
    TitleScene(SDL_Renderer* renderer);
    virtual ~TitleScene();

    virtual void init() override;
    virtual void update(const Inputs* inputs) override;
    virtual void draw(const Fps* fps, int w, int h) override;

    virtual std::unique_ptr<Scene> nextScene() const override;

private:
    Mix_Music*   music;
    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;
    SDL_Surface* startSurface;
    SDL_Texture* startTexture;

    bool goToNextScene = false;
    bool alternate     = false;
};

#endif // TITLESCENE_H
