#ifndef KEYBOARDSCENE_H
#define KEYBOARDSCENE_H

#include "keyboard.h"
#include "scene.h"

class KeyboardScene : public Scene
{
public:
    KeyboardScene(SDL_Renderer* renderer, std::string& textOut, size_t maxSize = 8);
    virtual ~KeyboardScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    std::unique_ptr<Keyboard> keyboard;
};

#endif // KEYBOARDSCENE_H
