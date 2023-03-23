#ifndef INTROSCENE_H
#define INTROSCENE_H

#include "scene.h"
#include "textspeech.h"
#include "textquestion.h"
#include "animations/introanimation.h"
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

    virtual void debug() override;

private:
    enum State
    {
        IntroSpeech,
        Keyboard,
        GenderSpeech,
        GenderQuestion,
        OutroSpeech,
        Leave
    };

    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;

    State state = IntroSpeech;

    std::unique_ptr<TextSpeech>   introSpeech;
    std::unique_ptr<TextSpeech>   genderSpeech;
    std::unique_ptr<TextQuestion> genderQuestion;
    std::unique_ptr<TextSpeech>   outroSpeech;

    std::unique_ptr<IntroAnimation> introAnimation;
};

#endif // INTROSCENE_H
