#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "scene.h"
#include <string>

class Keyboard : public Scene
{
public:
    Keyboard(SDL_Renderer* renderer, std::string& playerName);
    virtual ~Keyboard();

    virtual void init() override;
    virtual void update(const Inputs* inputs) override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    bool leave = false;

    std::string& playerName;
};

#endif // KEYBOARD_H
