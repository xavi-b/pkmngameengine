#ifndef BATTLE_H
#define BATTLE_H

#include "scene.h"

class Battle : public Scene
{
public:
    Battle(SDL_Renderer* renderer);

    virtual void sendInputs(const Inputs* inputs) override;
    virtual void draw(const Fps* fps) override;

    virtual std::unique_ptr<Scene> nextScene() const override;

private:
    bool goToNextScene = false;
};

#endif // BATTLE_H
