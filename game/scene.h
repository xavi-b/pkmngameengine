#ifndef SCENE_H
#define SCENE_H

#include "renderutils.h"
#include "inputs.h"
#include "fps.h"
#include <SDL.h>
#include <memory>

class Scene
{
public:
    Scene(SDL_Renderer* renderer);
    virtual ~Scene();

    virtual void init();
    virtual void update(const Inputs* inputs)         = 0;
    virtual void draw(const Fps* fps, RenderSizes rs) = 0;

    virtual bool popScene() const;
    virtual bool pushScene() const;

    virtual std::unique_ptr<Scene> nextScene();

    virtual std::string name() = 0;

    virtual void debug();

protected:
    SDL_Renderer* renderer;
};

#endif // SCENE_H
