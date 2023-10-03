#ifndef PKMNSSCENE_H
#define PKMNSSCENE_H

#include "scene.h"

#include <SDL_image.h>

class PkmnsScene : public Scene
{
public:
    PkmnsScene(SDL_Renderer* renderer);
    virtual ~PkmnsScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string name() override;

private:
};

#endif // PKMNSSCENE_H
