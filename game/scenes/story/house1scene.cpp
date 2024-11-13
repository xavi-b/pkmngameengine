#include "house1scene.h"

#include "game.h"
#include "scenes/story/town1scene.h"

House1Scene::House1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/house1.pkmap")
{
}

void House1Scene::init()
{
    MapScene::init();
}

void House1Scene::update(Inputs const* inputs)
{
    MapScene::update(inputs);
}

void House1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

bool House1Scene::manageEvents()
{
    auto& player = Game::instance()->data.player;
    auto& layer  = map->getLevels()[player.l]->getEventLayer();
    auto& event  = (*layer.get())(player.x, player.y);

    if (event && event->getId() == "Town1")
    {
        if (player.direction == Entity::Direction::DOWN)
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

std::string House1Scene::name()
{
    return "House1Scene";
}

std::unique_ptr<Scene> House1Scene::nextScene()
{
    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (fadeOutAnimation->isStarted() && fadeOutAnimation->isFinished())
    {
        if (goToScene == "Town1")
        {
            auto scene = std::make_unique<Town1Scene>(renderer);
            scene->initMovingPlayerPosition(8, 12, 0, Entity::Direction::DOWN);
            scene->initClosingDoor(8, 11);
            return scene;
        }
    }

    return nullptr;
}

bool House1Scene::shouldShowNightTextures() const
{
    return false;
}
