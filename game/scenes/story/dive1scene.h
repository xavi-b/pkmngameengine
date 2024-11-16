#ifndef DIVE1SCENE_H
#define DIVE1SCENE_H

#include "animations/weather/bubblesanimation.h"
#include "scenes/mapscene.h"

#include <memory>
#include <string>

class Dive1Scene : public MapScene
{
public:
    Dive1Scene(SDL_Renderer* renderer);

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual void drawAmbientOverlay(Fps const* fps, RenderSizes rs, size_t offsetX, size_t offsetY) override;

    virtual bool manageEvents() override;

    virtual std::string name() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual bool shouldShowNightTextures() const override;

private:
    std::unique_ptr<BubblesAnimation> bubblesAnimation;
};

#endif // DIVE1SCENE_H
