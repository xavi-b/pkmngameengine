#include "house1scene.h"

#include "animations/map/stairsanimation.h"
#include "game.h"
#include "scenes/story/town1scene.h"

House1Scene::House1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/house1.pkmap")
{
}

void House1Scene::init()
{
    MapScene::init();
}

bool House1Scene::updateBeforeMovement(Inputs const* /*inputs*/)
{
    auto& player = Game::instance()->data.player;

    auto event = eventAt(player.x, player.y, player.l);

    if (event)
    {
        if (event->getId() == "Town1")
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
                else
                {
                    stopAfterLastMovementFrame(player);
                }
            }
        }
    }

    return false;
}

void House1Scene::updateAfterMovement(Inputs const* /*inputs*/)
{
    auto& player = Game::instance()->data.player;

    if (auto event = facedPreviousEvent(player))
    {
        if (event->getId() == "UpStairsEntrance")
        {
            if (player.direction == Entity::Direction::RIGHT)
            {
                if (!stairsExitAnimation)
                {
                    stairsExitAnimation = std::make_unique<StairsAnimation>(renderer,
                                                                            StairsAnimation::ToUpstairs,
                                                                            shouldShowNightTextures());
                    stairsExitAnimation->start();
                    stairsExitPosition = {player.x + 1, player.y};
                    goToScene          = "UpStairsEntrance";
                    player.direction   = Entity::Direction::RIGHT;
                    move(player, true);
                }
            }
        }

        if (event->getId() == "DownStairsEntrance")
        {
            if (player.direction == Entity::Direction::RIGHT)
            {
                if (!stairsExitAnimation)
                {
                    stairsExitAnimation = std::make_unique<StairsAnimation>(renderer,
                                                                            StairsAnimation::ToDownstairs,
                                                                            shouldShowNightTextures());
                    stairsExitAnimation->start();
                    stairsExitPosition = {player.x + 1, player.y};
                    goToScene          = "DownStairsEntrance";
                    player.direction   = Entity::Direction::RIGHT;
                    move(player, true);
                }
            }
        }
    }
}

void House1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
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

        if (goToScene == "UpStairsEntrance")
        {
            auto scene = std::make_unique<House1Scene>(renderer);
            scene->initMovingPlayerPosition(22, 4, 0, Entity::Direction::RIGHT);
            scene->initStairsEntrance(StairsAnimation::FromDownstairs);
            return scene;
        }

        if (goToScene == "DownStairsEntrance")
        {
            auto scene = std::make_unique<House1Scene>(renderer);
            scene->initMovingPlayerPosition(15, 4, 0, Entity::Direction::RIGHT);
            scene->initStairsEntrance(StairsAnimation::FromUpstairs);
            return scene;
        }
    }

    return nullptr;
}

bool House1Scene::shouldShowNightTextures() const
{
    return false;
}

bool House1Scene::allowBike() const
{
    return false;
}

std::string House1Scene::currentLocation() const
{
    return lc::translate("House 1");
}
