#include "cave3scene.h"

#include "game.h"
#include "scenes/story/cave2scene.h"

Cave3Scene::Cave3Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/cave3.pkmap")
{
}

void Cave3Scene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    MapScene::update(inputs);

    if (auto event = eventAt(player.x, player.y, player.l))
    {
        if (event->getId() == "Cave2" && player.direction != Entity::Direction::NONE)
        {
            if (!fadeOutAnimation->isStarted())
            {
                fadeOutAnimation->reset();
                fadeOutAnimation->start();
                goToScene = "Cave2";
            }
            else
            {
                if (playerSprite->getAccumulatedTicks() == 0)
                    stop(player);
            }
        }
    }
}

void Cave3Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

std::string Cave3Scene::name()
{
    return "Cave3Scene";
}

std::unique_ptr<Scene> Cave3Scene::nextScene()
{
    auto& player = Game::instance()->data.player;

    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (fadeOutAnimation->isStarted() && fadeOutAnimation->isFinished())
    {
        if (goToScene == "Cave2")
        {
            auto scene = std::make_unique<Cave2Scene>(renderer);
            scene->initPlayerPosition(player.x, player.y, 0, player.direction);
            return scene;
        }
    }
    return nullptr;
}

bool Cave3Scene::shouldShowNightTextures() const
{
    return false;
}

std::string Cave3Scene::currentLocation() const
{
    return "";
}
