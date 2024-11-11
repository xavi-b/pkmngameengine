#ifndef HOUSE1SCENE_H
#define HOUSE1SCENE_H

#include "scenes/mapscene.h"

#include <memory>
#include <string>

class House1Scene : public MapScene
{
public:
    House1Scene(SDL_Renderer* renderer);

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool manageEvents() override;

    virtual std::string name() override;

    virtual std::unique_ptr<Scene> nextScene() override;
};

#endif // HOUSE1SCENE_H
