#include "cave2scene.h"

#include "game.h"
#include "scenes/story/cave3scene.h"
#include "scenes/story/road2scene.h"

Cave2Scene::Cave2Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/cave2.pkmap")
{
}

void Cave2Scene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    if (!preventInputs)
    {
        if (auto event = facedPreviousEvent(player))
        {
            if (event->getId() == "Road2")
            {
                if (player.direction == Entity::Direction::DOWN)
                {
                    if (!fadeOutAnimation->isStarted())
                    {
                        fadeOutAnimation->reset();
                        fadeOutAnimation->start();
                        goToScene = "Road2";
                    }
                }
            }
        }
    }

    MapScene::update(inputs);

    if (auto event = eventAt(player.x, player.y, player.l))
    {
        if (event->getId() == "Cave3" && player.direction != Entity::Direction::NONE)
        {
            if (!fadeOutAnimation->isStarted())
            {
                fadeOutAnimation->reset();
                fadeOutAnimation->start();
                goToScene = "Cave3";
            }
            else
            {
                if (playerSprite->getAccumulatedTicks() == 0)
                    stop(player);
            }
        }
    }
}

void Cave2Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

std::string Cave2Scene::name()
{
    return "Cave2Scene";
}

std::unique_ptr<Scene> Cave2Scene::nextScene()
{
    auto& player = Game::instance()->data.player;

    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (fadeOutAnimation->isStarted() && fadeOutAnimation->isFinished())
    {
        if (goToScene == "Road2")
        {
            auto scene = std::make_unique<Road2Scene>(renderer);
            scene->initMovingPlayerPosition(19, 3, 0, Entity::Direction::DOWN);
            return scene;
        }

        if (goToScene == "Cave3")
        {
            auto scene = std::make_unique<Cave3Scene>(renderer);
            scene->initPlayerPosition(player.x, player.y, 0, player.direction);
            return scene;
        }
    }
    return nullptr;
}

bool Cave2Scene::shouldShowNightTextures() const
{
    return false;
}

std::string Cave2Scene::currentLocation() const
{
    return lc::translate("Cave 2");
}
