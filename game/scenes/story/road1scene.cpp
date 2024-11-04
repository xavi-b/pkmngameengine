#include "road1scene.h"

#include "game.h"
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

bool Road1Scene::manageEvents()
{
    auto& layer = map->getLevels()[Game::instance()->data.player.l]->getEventLayer();
    auto& event = (*layer.get())(Game::instance()->data.player.x, Game::instance()->data.player.y);
    if (event && event->getId() == "Town1")
    {
        if (Game::instance()->data.player.direction == Entity::Direction::LEFT)
        {
            if (!fadeOutAnimation->isStarted())
            {
                fadeOutAnimation->reset();
                fadeOutAnimation->start();
                goToScene = "Town1";
                return true;
            }
        }
    }

    return MapScene::manageEvents();
}

std::string Road1Scene::name()
{
    return "Road1Scene";
}

std::unique_ptr<Scene> Road1Scene::nextScene()
{
    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (fadeOutAnimation->isStarted() && fadeOutAnimation->isFinished())
    {
        if (goToScene == "Town1")
        {
            auto town1Scene = std::make_unique<Town1Scene>(renderer);
            town1Scene->initPlayerPosition(19, 2, Entity::Direction::LEFT);
            return town1Scene;
        }
    }
    return nullptr;
}
