#include "scene.h"

Scene::Scene(SDL_Renderer* renderer)
    : renderer(renderer)
{
}

Scene::~Scene()
{
}

std::unique_ptr<Scene> Scene::nextScene() const
{
    return nullptr;
}
