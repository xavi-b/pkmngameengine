#include "town1scene.h"

#include "game.h"
#include "scenes/story/road1scene.h"

Town1Scene::Town1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/town1.pkmap")
{
    auto entity       = std::make_unique<Entity>();
    entity->x         = 11;
    entity->y         = 11;
    auto entitySprite = std::make_unique<Sprite>(renderer);
    entitySprite->load("resources/Graphics/Characters/NPC 01.png");
    entities.emplace(std::move(entity), std::move(entitySprite));
}

void Town1Scene::init()
{
}

void Town1Scene::update(Inputs const* inputs)
{
    MapScene::update(inputs);

    auto& entity = *(entities.begin()->first.get());
    entity.x     = 11;
    entity.y     = 11;
    // entity.direction = Entity::RIGHT;
    // move(entity);
}

void Town1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

bool Town1Scene::manageEvents()
{
    auto& layer = map->getLevels()[Game::instance()->data.player.l]->getEventLayer();
    auto& event = (*layer.get())(Game::instance()->data.player.x, Game::instance()->data.player.y);
    if (event && event->getId() == "Road1")
    {
        if (Game::instance()->data.player.direction == Entity::Direction::RIGHT)
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
            auto road1Scene = std::make_unique<Road1Scene>(renderer);
            road1Scene->initPlayerPosition(0, 2, Entity::Direction::RIGHT);
            return road1Scene;
        }
    }

    return nullptr;
}
