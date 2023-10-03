#ifndef BAGSCENE_H
#define BAGSCENE_H

#include "scene.h"

#include <SDL_image.h>

class BagScene : public Scene
{
public:
    BagScene(SDL_Renderer* renderer);
    virtual ~BagScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string name() override;

private:
};

#endif // BAGSCENE_H
