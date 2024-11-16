#include "dive1scene.h"

#include "game.h"
#include "scenes/story/road1scene.h"

Dive1Scene::Dive1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/dive1.pkmap")
{
    diving = true;
}

void Dive1Scene::init()
{
    MapScene::init();

    bubblesAnimation = std::make_unique<BubblesAnimation>(renderer, shouldShowNightTextures());
    bubblesAnimation->start();
}

void Dive1Scene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    MapScene::update(inputs);

    bubblesAnimation->incrementTicks();

    if (!preventInputs)
    {
        if (inputs->A)
        {
            auto event = eventAt(player.x, player.y, player.l);
            if (event && event->getId() == "Road1")
            {
                if (!fadeOutAnimation->isStarted())
                {
                    fadeOutAnimation->reset();
                    fadeOutAnimation->start();
                    goToScene = "Road1";
                }
                return;
            }
        }
    }
}

void Dive1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

void Dive1Scene::drawAmbientOverlay(Fps const* fps, RenderSizes rs, size_t offsetX, size_t offsetY)
{
    (void)offsetX;
    (void)offsetY;
    bubblesAnimation->draw(fps, rs);
}

bool Dive1Scene::manageEvents()
{
    return MapScene::manageEvents();
}

std::string Dive1Scene::name()
{
    return "Dive1Scene";
}

std::unique_ptr<Scene> Dive1Scene::nextScene()
{
    auto& player = Game::instance()->data.player;

    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (fadeOutAnimation->isStarted() && fadeOutAnimation->isFinished())
    {
        if (goToScene == "Road1")
        {
            int  offsetX = player.x - 1;
            int  offsetY = player.y - 1;
            auto scene   = std::make_unique<Road1Scene>(renderer);
            scene->initPlayerPosition(18 + offsetX, 18 + offsetY, 0, player.direction);
            return scene;
        }
    }
    return nullptr;
}

bool Dive1Scene::shouldShowNightTextures() const
{
    return false;
}
