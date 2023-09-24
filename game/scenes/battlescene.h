#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "scene.h"
#include "textquestion.h"
#include "textspeech.h"

#include <SDL_image.h>

class BattleScene : public Scene
{
public:
    BattleScene(SDL_Renderer* renderer);
    virtual ~BattleScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    std::unique_ptr<TextSpeech>   quitSpeech;
    std::unique_ptr<TextQuestion> quitQuestion;
};

#endif // BATTLESCENE_H
