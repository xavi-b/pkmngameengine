#ifndef INTROSCENE_H
#define INTROSCENE_H

#include "scene.h"
#include "textspeech.h"
#include <SDL_image.h>

class IntroScene : public Scene
{
public:
    IntroScene(SDL_Renderer* renderer);
    virtual ~IntroScene();

    virtual void init() override;
    virtual void update(const Inputs* inputs) override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;

    virtual bool pushScene() const override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;

    bool showKeyboardScene = false;

    std::unique_ptr<TextSpeech> speech;
};

#endif // INTROSCENE_H
