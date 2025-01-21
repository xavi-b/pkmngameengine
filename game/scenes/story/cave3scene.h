#ifndef CAVE3SCENE_H
#define CAVE3SCENE_H

#include "scenes/mapscene.h"

#include <memory>
#include <string>

class Cave3Scene : public MapScene
{
public:
    Cave3Scene(SDL_Renderer* renderer);

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string name() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual bool shouldShowNightTextures() const override;

    virtual std::string currentLocation() const override;
};

#endif // CAVE3SCENE_H
