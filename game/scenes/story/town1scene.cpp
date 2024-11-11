#include "town1scene.h"

#include "game.h"
#include "scenes/story/house1scene.h"
#include "scenes/story/road1scene.h"

Town1Scene::Town1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/town1.pkmap")
{
    auto entity               = std::make_unique<Entity>();
    entity->x                 = 11;
    entity->y                 = 10;
    entity->previousX         = 11;
    entity->previousY         = 10;
    entity->direction         = Entity::Direction::RIGHT;
    entity->previousDirection = Entity::Direction::RIGHT;
    auto entitySprite         = std::make_unique<Sprite>(renderer);
    entitySprite->load("resources/Graphics/Characters/NPC 01.png");
    entities.emplace(std::move(entity), std::move(entitySprite));
}

void Town1Scene::init()
{
    changeWeather(Map::Weather::RAIN);
}

void Town1Scene::update(Inputs const* inputs)
{
    MapScene::update(inputs);

    auto& player = Game::instance()->data.player;
    if (doorAnimation)
    {
        if (player.x != doorPosition.first || player.y != doorPosition.second)
        {
            if (goToScene == "House1")
            {
                player.direction = Entity::Direction::UP;
                move(player, true);
            }
        }
        else
        {
            if (playerSprite->getAccumulatedTicks() == 0)
                stop(player);
        }

        if (doorAnimation->isFinished())
        {
            if (!fadeOutAnimation->isStarted())
            {
                fadeOutAnimation->reset();
                fadeOutAnimation->start();
            }
        }
    }

    auto& entity = *(entities.begin()->first.get());
    auto& sprite = *(entities.begin()->second.get());
    if (sprite.getAccumulatedTicks() == 0)
    {
        if (entity.x == 18)
        {
            entity.direction = Entity::Direction::LEFT;
        }
        else if (entity.x == 11)
        {
            entity.direction = Entity::Direction::RIGHT;
        }
        move(entity);
    }
}

void Town1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

bool Town1Scene::manageEvents()
{
    auto& player = Game::instance()->data.player;
    auto& layer  = map->getLevels()[player.l]->getEventLayer();
    auto& event  = (*layer.get())(player.x, player.y);

    if (event && event->getId() == "Road1")
    {
        if (player.direction == Entity::Direction::RIGHT)
        {
            if (!fadeOutAnimation->isStarted())
            {
                fadeOutAnimation->reset();
                fadeOutAnimation->start();
                goToScene = "Road1";
                return true;
            }
        }
    }

    if (event && event->getId() == "House1")
    {
        if (player.direction == Entity::Direction::UP)
        {
            if (!doorAnimation)
            {
                doorAnimation = std::make_unique<DoorAnimation>(renderer);
                doorAnimation->start();
                doorPosition = {player.x, player.y - 1};
                goToScene    = "House1";
            }

            return true;
        }
    }

    return MapScene::manageEvents();
}

std::string Town1Scene::name()
{
    return "Town1Scene";
}

std::unique_ptr<Scene> Town1Scene::nextScene()
{
    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (fadeOutAnimation->isStarted() && fadeOutAnimation->isFinished())
    {
        if (goToScene == "Road1")
        {
            auto scene = std::make_unique<Road1Scene>(renderer);
            scene->initPlayerPosition(0, 2, Entity::Direction::RIGHT);
            return scene;
        }

        if (goToScene == "House1")
        {
            auto scene = std::make_unique<House1Scene>(renderer);
            scene->initPlayerPosition(1, 7, Entity::Direction::UP);
            return scene;
        }
    }

    return nullptr;
}
