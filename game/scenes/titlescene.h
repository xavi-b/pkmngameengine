#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "renderutils.h"
#include "scene.h"

#include <SDL_image.h>
#include <SDL_mixer.h>

class TitleScene : public Scene
{
public:
    TitleScene(SDL_Renderer* renderer);
    virtual ~TitleScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    Mix_Music*   music;
    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;
    SDL_Surface* startSurface;
    SDL_Texture* startTexture;

    bool goToNextScene = false;
    int  ticks         = 0;
};

#endif // TITLESCENE_H
