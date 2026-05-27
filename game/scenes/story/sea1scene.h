#ifndef SEA1SCENE_H
#define SEA1SCENE_H

#include "scenes/mapscene.h"

#include <memory>
#include <string>

class Sea1Scene : public MapScene
{
public:
    Sea1Scene(SDL_Renderer* renderer);

    virtual std::string name() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual bool shouldShowNightTextures() const override;

    virtual std::string currentLocation() const override;

protected:
    virtual void updateAfterMovement(Inputs const* inputs) override;
};

#endif // SEA1SCENE_H
