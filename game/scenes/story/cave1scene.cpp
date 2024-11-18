#include "cave1scene.h"

#include "game.h"
#include "scenes/story/road1scene.h"

Cave1Scene::Cave1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/cave1.pkmap")
{
}

void Cave1Scene::init()
{
    MapScene::init();
    flash = false;
}

void Cave1Scene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    if (!preventInputs)
    {
        if (auto event = facedPreviousEvent(player))
        {
            if (event->getId() == "Road1")
            {
                if (player.direction == Entity::Direction::DOWN)
                {
                    if (!fadeOutAnimation->isStarted())
                    {
                        fadeOutAnimation->reset();
                        fadeOutAnimation->start();
                        goToScene = "Road1";
                    }
                }
            }
        }
    }

    MapScene::update(inputs);
}

void Cave1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

bool Cave1Scene::manageEvents()
{
    auto& player = Game::instance()->data.player;
    auto& layer  = map->getLevels()[player.l]->getEventLayer();
    auto& event  = (*layer.get())(player.x, player.y);
    if (event)
    {
        if (event->getId() == "TurnOnFlash")
        {
            if (turnOnFlash())
                return true;
        }
    }

    return MapScene::manageEvents();
}

std::string Cave1Scene::name()
{
    return "Cave1Scene";
}

std::unique_ptr<Scene> Cave1Scene::nextScene()
{
    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (fadeOutAnimation->isStarted() && fadeOutAnimation->isFinished())
    {
        if (goToScene == "Road1")
        {
            auto scene = std::make_unique<Road1Scene>(renderer);
            scene->initPlayerPosition(17, 10, 0, Entity::Direction::DOWN);
            return scene;
        }
    }
    return nullptr;
}

bool Cave1Scene::shouldShowNightTextures() const
{
    return false;
}
