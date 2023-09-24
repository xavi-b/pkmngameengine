#ifndef LEAVESCENE_H
#define LEAVESCENE_H

#include "scene.h"
#include "textquestion.h"
#include "textspeech.h"

#include <SDL_image.h>

class LeaveScene : public Scene
{
public:
    LeaveScene(SDL_Renderer* renderer);
    virtual ~LeaveScene();

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

#endif // LEAVESCENE_H
