#include "scene.h"

Scene::Scene(SDL_Renderer* renderer)
    : renderer(renderer)
{
}

Scene::~Scene()
{
}

void Scene::init()
{
}

bool Scene::popScene() const
{
    return false;
}

bool Scene::pushScene() const
{
    return false;
}

std::unique_ptr<Scene> Scene::nextScene()
{
    return nullptr;
}
