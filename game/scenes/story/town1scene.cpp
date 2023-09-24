#include "town1scene.h"

#include "scenes/story/road1scene.h"

Town1Scene::Town1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/town1.pkmap")
{
}

void Town1Scene::init()
{
}

void Town1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

std::string Town1Scene::name()
{
    return "Town1Scene";
}

std::unique_ptr<Scene> Town1Scene::nextScene()
{
    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    auto& layer = map->getLevels()[playerLevel]->getEventLayer();
    auto& event = (*layer.get())(playerX, playerY);
    if (event && event->getId() == "Road1")
    {
        if (direction == RIGHT)
        {
            auto road1Scene = std::make_unique<Road1Scene>(renderer);
            road1Scene->initPlayerPosition(0, 2, RIGHT);
            return road1Scene;
        }
    }
    return nullptr;
}
