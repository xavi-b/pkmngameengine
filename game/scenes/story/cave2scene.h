#ifndef CAVE2SCENE_H
#define CAVE2SCENE_H

#include "scenes/mapscene.h"

#include <memory>
#include <string>

class Cave2Scene : public MapScene
{
public:
    Cave2Scene(SDL_Renderer* renderer);

    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string name() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual bool shouldShowNightTextures() const override;

    virtual std::string currentLocation() const override;

protected:
    virtual bool updateBeforeMovement(Inputs const* inputs) override;
    virtual void updateAfterMovement(Inputs const* inputs) override;
};

#endif // CAVE2SCENE_H
