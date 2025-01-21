#include "road2scene.h"

#include "game.h"
#include "scenes/story/cave2scene.h"
#include "scenes/story/road1scene.h"
#include "scenes/story/sea1scene.h"

Road2Scene::Road2Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/road2.pkmap")
{
}

void Road2Scene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    MapScene::update(inputs);

    if (!preventInputs)
    {
        if (player.x == 8 && player.direction == Entity::Direction::LEFT)
        {
            goToScene = "Road1";
        }

        if (player.x > 17 && player.x < 22 && player.y == 17 && player.direction == Entity::Direction::DOWN)
        {
            goToScene = "Sea1";
        }

        if (auto event = facedPreviousEvent(player))
        {
            if (event->getId() == "Cave2")
            {
                if (player.direction == Entity::Direction::UP)
                {
                    if (!fadeOutAnimation->isStarted())
                    {
                        fadeOutAnimation->reset();
                        fadeOutAnimation->start();
                        goToScene = "Cave2";
                    }
                }
            }
        }
    }
}

std::string Road2Scene::name()
{
    return "Road2Scene";
}

std::unique_ptr<Scene> Road2Scene::nextScene()
{
    auto& player = Game::instance()->data.player;

    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (goToScene == "Road1")
    {
        auto scene = std::make_unique<Road1Scene>(renderer);
        scene->initMovingPlayerPosition(22, player.y, 0, Entity::Direction::LEFT, false);
        return scene;
    }

    if (goToScene == "Sea1")
    {
        auto scene = std::make_unique<Sea1Scene>(renderer);
        scene->initMovingSurfingPlayerPosition(player.x - 10, 9, 0, Entity::Direction::DOWN, false);
        return scene;
    }

    if (fadeOutAnimation->isStarted() && fadeOutAnimation->isFinished())
    {
        if (goToScene == "Cave2")
        {
            auto scene = std::make_unique<Cave2Scene>(renderer);
            scene->initPlayerPosition(6, 7, 0, Entity::Direction::UP);
            return scene;
        }
    }

    return nullptr;
}

bool Road2Scene::shouldShowNightTextures() const
{
    return Game::instance()->isNight();
}

std::string Road2Scene::currentLocation() const
{
    return lc::translate("Road 2");
}
