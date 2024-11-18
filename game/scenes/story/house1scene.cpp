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

void House1Scene::update(Inputs const* inputs)
{
    MapScene::update(inputs);

    auto& player = Game::instance()->data.player;

    if (stairsExitAnimation)
    {
        if (player.x == stairsExitPosition.first && player.y == stairsExitPosition.second)
        {
            if (playerSprite->getAccumulatedTicks() == 0)
                stop(player);
        }
    }

    if (stairsEntranceAnimation && !stairsEntranceAnimation->isFinished())
    {
        if (playerSprite->getAccumulatedTicks() == 0)
            stop(player);
    }

    if (!preventInputs)
    {
        if (auto event = facedPreviousEvent(player))
        {
            if (event->getId() == "UpStairsEntrance")
            {
                if (!stairsExitAnimation)
                {
                    stairsExitAnimation = std::make_unique<StairsAnimation>(renderer,
                                                                            StairsAnimation::ToUpstairs,
                                                                            shouldShowNightTextures());
                    stairsExitAnimation->start();
                    stairsExitPosition = {player.x + 1, player.y};
                    preventInputs      = true;
                    goToScene          = "UpStairsEntrance";
                    player.direction   = Entity::Direction::RIGHT;
                    move(player, true);
                }
            }

            if (event->getId() == "DownStairsEntrance")
            {
                if (!stairsExitAnimation)
                {
                    stairsExitAnimation = std::make_unique<StairsAnimation>(renderer,
                                                                            StairsAnimation::ToDownstairs,
                                                                            shouldShowNightTextures());
                    stairsExitAnimation->start();
                    stairsExitPosition = {player.x + 1, player.y};
                    preventInputs      = true;
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

        if (goToScene == "UpStairsEntrance")
        {
            auto scene = std::make_unique<House1Scene>(renderer);
            scene->initMovingPlayerPosition(22, 4, 0, Entity::Direction::RIGHT);
            scene->initStairsEntrance(StairsAnimation::FromUpstairs);
            return scene;
        }

        if (goToScene == "DownStairsEntrance")
        {
            auto scene = std::make_unique<House1Scene>(renderer);
            scene->initMovingPlayerPosition(15, 4, 0, Entity::Direction::RIGHT);
            scene->initStairsEntrance(StairsAnimation::FromDownstairs);
            return scene;
        }
    }

    return nullptr;
}

bool House1Scene::shouldShowNightTextures() const
{
    return false;
}
