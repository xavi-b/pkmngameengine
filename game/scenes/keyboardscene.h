#ifndef KEYBOARDSCENE_H
#define KEYBOARDSCENE_H

#include "scene.h"
#include "keyboard.h"

class KeyboardScene : public Scene
{
public:
    KeyboardScene(SDL_Renderer* renderer, std::string& textOut, size_t maxSize = 8);
    virtual ~KeyboardScene();

    virtual void init() override;
    virtual void update(const Inputs* inputs) override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    std::unique_ptr<Keyboard> keyboard;
};

#endif // KEYBOARDSCENE_H
