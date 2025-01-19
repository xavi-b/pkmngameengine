#ifndef ROAD2SCENE_H
#define ROAD2SCENE_H

#include "scenes/mapscene.h"

#include <memory>
#include <string>

class Road2Scene : public MapScene
{
public:
    Road2Scene(SDL_Renderer* renderer);

    virtual void update(Inputs const* inputs) override;

    virtual std::string name() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual bool shouldShowNightTextures() const override;

    virtual std::string currentLocation() const override;

private:
};

#endif // ROAD2SCENE_H
