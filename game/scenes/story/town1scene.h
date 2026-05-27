#ifndef TOWN1SCENE_H
#define TOWN1SCENE_H

#include "scenes/mapscene.h"

#include <memory>
#include <string>

class Town1Scene : public MapScene
{
public:
    Town1Scene(SDL_Renderer* renderer);

    virtual void init() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string name() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual bool shouldShowNightTextures() const override;

    virtual std::string currentLocation() const override;

protected:
    virtual bool updateBeforeMovement(Inputs const* inputs) override;
    virtual void updateAfterMovement(Inputs const* inputs) override;
};

#endif // TOWN1SCENE_H
