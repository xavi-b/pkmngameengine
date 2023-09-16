#include "road1scene.h"
#include "scenes/story/town1scene.h"

Road1Scene::Road1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/road1.pkmap")
{
}

void Road1Scene::init()
{
}

void Road1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

std::string Road1Scene::name()
{
    return "Road1Scene";
}

std::unique_ptr<Scene> Road1Scene::nextScene()
{
    auto& layer = map->getLevels()[playerLevel]->getEventLayer();
    auto& event = (*layer.get())(playerX, playerY);
    if (event && event->getId() == "Town1")
    {
        if (direction == LEFT)
        {
            auto town1Scene = std::make_unique<Town1Scene>(renderer);
            town1Scene->initPlayerPosition(19, 2);
            return town1Scene;
        }
    }
    return nullptr;
}
