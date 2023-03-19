#ifndef BATTLE_H
#define BATTLE_H

#include "scene.h"

class Battle : public Scene
{
public:
    Battle(SDL_Renderer* renderer);

    virtual void update(const Inputs* inputs) override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    bool goToNextScene = false;
};

#endif // BATTLE_H
