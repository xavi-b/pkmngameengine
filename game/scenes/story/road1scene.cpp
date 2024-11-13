#include "road1scene.h"

#include "game.h"
#include "scenes/story/cave1scene.h"
#include "scenes/story/town1scene.h"

Road1Scene::Road1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/road1.pkmap")
{
}

void Road1Scene::init()
{
    MapScene::init();

    fogAnimation = std::make_unique<FogAnimation>(renderer, shouldShowNightTextures());
    fogAnimation->start();
}

void Road1Scene::update(Inputs const* inputs)
{
    fogAnimation->incrementTicks();
    MapScene::update(inputs);
}

void Road1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

void Road1Scene::drawAmbientOverlay(Fps const* fps, RenderSizes rs, size_t offsetX, size_t offsetY)
{
    fogAnimation->setOffsetX(offsetX);
    fogAnimation->setOffsetY(offsetY);
    // TODO temp fogAnimation->draw(fps, rs);
    (void)fps;
    (void)rs;
}

bool Road1Scene::manageEvents()
{
    auto& player = Game::instance()->data.player;
    auto& layer  = map->getLevels()[player.l]->getEventLayer();
    auto& event  = (*layer.get())(player.x, player.y);
    if (event)
    {
        if (event->getId() == "Town1")
        {
            if (player.direction == Entity::Direction::LEFT)
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

        if (event->getId() == "Cave1")
        {
            if (player.direction == Entity::Direction::UP)
            {
                if (!fadeOutAnimation->isStarted())
                {
                    fadeOutAnimation->reset();
                    fadeOutAnimation->start();
                    goToScene = "Cave1";
                    return true;
                }
            }
        }

        if (event->getId() == "Up1")
        {
            if (player.direction == Entity::Direction::UP)
            {
                player.l++;
            }
        }

        if (event->getId() == "Down1")
        {
            if (player.direction == Entity::Direction::DOWN)
            {
                player.l--;
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
            auto scene = std::make_unique<Town1Scene>(renderer);
            scene->initPlayerPosition(19, 2, 0, Entity::Direction::LEFT);
            return scene;
        }

        if (goToScene == "Cave1")
        {
            auto scene = std::make_unique<Cave1Scene>(renderer);
            scene->initPlayerPosition(6, 7, 0, Entity::Direction::UP);
            return scene;
        }
    }
    return nullptr;
}

bool Road1Scene::shouldShowNightTextures() const
{
    return Game::instance()->isNight();
}
