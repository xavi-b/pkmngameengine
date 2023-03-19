#ifndef SCENE_H
#define SCENE_H

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
    virtual void update(const Inputs* inputs)       = 0;
    virtual void draw(const Fps* fps, int w, int h) = 0;

    virtual std::unique_ptr<Scene> nextScene() const;

protected:
    SDL_Renderer* renderer;
};

#endif // SCENE_H
