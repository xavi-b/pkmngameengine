#include "road1scene.h"

#include "game.h"
#include "scenes/story/cave1scene.h"
#include "scenes/story/dive1scene.h"
#include "scenes/story/road2scene.h"
#include "scenes/story/town1scene.h"

Road1Scene::Road1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/road1.pkmap")
{
}

void Road1Scene::init()
{
    MapScene::init();

    fogAnimation = std::make_unique<FogAnimation>(renderer, shouldShowNightTextures());
    fogAnimation->start();

    {
        auto entity       = std::make_unique<Entity>();
        childNpc          = entity.get();
        entity->x         = 4;
        entity->y         = 6;
        entity->previousX = 4;
        entity->previousY = 6;
        entity->direction = Entity::Direction::RIGHT;
        auto entitySprite = std::make_unique<Sprite>(renderer);
        entitySprite->load("resources/Graphics/Characters/NPC 02.png", shouldShowNightTextures());
        childSprite = entitySprite.get();
        entities.emplace(std::move(entity), std::move(entitySprite));
    }

    {
        auto entity       = std::make_unique<Entity>();
        ladyNpc           = entity.get();
        entity->x         = 3;
        entity->y         = 7;
        entity->previousX = 3;
        entity->previousY = 7;
        entity->direction = Entity::Direction::DOWN;
        auto entitySprite = std::make_unique<Sprite>(renderer);
        entitySprite->load("resources/Graphics/Characters/NPC 04.png", shouldShowNightTextures());
        entities.emplace(std::move(entity), std::move(entitySprite));
    }

    {
        auto entity       = std::make_unique<Entity>();
        entity->x         = 4;
        entity->y         = 10;
        entity->previousX = 4;
        entity->previousY = 10;
        entity->direction = Entity::Direction::NONE;
        auto entitySprite = std::make_unique<Sprite>(renderer);
        entitySprite->load("resources/Graphics/Characters/NPC 07.png", shouldShowNightTextures());
        entitySprite->forceSpriteDirection(Entity::Direction::UP);
        entities.emplace(std::move(entity), std::move(entitySprite));
    }

    text1Speech = std::make_unique<TextSpeech>(renderer);
    text1Speech->setTexts({lc::translate("First sign of the game !"), lc::translate("Impressive, right ?")});

    childSpeech = std::make_unique<TextSpeech>(renderer);
    childSpeech->setTexts({lc::translate("First NPC chat of the game !"), lc::translate("Are we cool ?")});
}

void Road1Scene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    fogAnimation->incrementTicks();

    if (text1Speech->isStarted() && !text1Speech->shouldClose())
    {
        text1Speech->update(inputs);
        preventInputs = true;
    }

    if (childSpeech->isStarted() && !childSpeech->shouldClose())
    {
        childSpeech->update(inputs);
        preventInputs = true;
    }

    if (!preventInputs)
    {
        if (inputs->A)
        {
            auto event = eventAt(player.x, player.y, player.l);
            if (event && event->getId() == "Dive1")
            {
                if (!fadeOutAnimation->isStarted())
                {
                    fadeOutAnimation->reset();
                    fadeOutAnimation->start();
                    goToScene = "Dive1";
                }
                return;
            }

            if (auto event = facedEvent(player))
            {
                if (event->getId() == "Text1")
                {
                    text1Speech->reset();
                    text1Speech->start();
                    preventInputs = true;
                }
            }
            else if (auto entity = facedEntity(player))
            {
                if (entity == childNpc)
                {
                    childSprite->forceSpriteDirection(Entity::getOppositeDirection(player.direction));
                    childNpc->previousDirection = childNpc->direction;
                    childSpeech->reset();
                    childSpeech->start();
                    preventInputs = true;
                }
            }
        }
    }

    MapScene::update(inputs);

    if (childSpeech->isStarted() && !childSpeech->shouldClose())
    {
        if (playerSprite->getAccumulatedTicks() == 0)
            stop(player);
    }

    if (text1Speech->isStarted() && !text1Speech->shouldClose())
    {
        if (playerSprite->getAccumulatedTicks() == 0)
            stop(player);
    }

    for (auto const& pair : entities)
    {
        auto entity = pair.first.get();
        auto sprite = pair.second.get();

        if (sprite->getAccumulatedTicks() == 0)
        {
            if (entity == childNpc)
            {
                if (childSpeech->isStarted() && !childSpeech->shouldClose()
                    && childNpc->direction != Entity::Direction::NONE)
                {
                    stop(*childNpc);
                }
                else if (!childSpeech->isStarted() || childSpeech->shouldClose())
                {
                    if (childNpc->direction == Entity::Direction::NONE)
                        childNpc->direction = childNpc->previousDirection;
                    childNpc->previousDirection = Entity::Direction::NONE;

                    if (childNpc->x == 10 || (childNpc->x != 4 && childNpc->direction == Entity::Direction::LEFT))
                    {
                        childNpc->direction = Entity::Direction::LEFT;
                    }
                    else if (childNpc->x == 4 || (childNpc->x != 10 && childNpc->direction == Entity::Direction::RIGHT))
                    {
                        childNpc->direction = Entity::Direction::RIGHT;
                    }
                    move(*childNpc);
                }
            }

            if (entity == ladyNpc)
            {
                if (ladyNpc->y == 7)
                {
                    ladyNpc->direction = Entity::Direction::DOWN;
                }
                else if (ladyNpc->y == 15)
                {
                    ladyNpc->direction = Entity::Direction::UP;
                }
                move(*ladyNpc);
            }
        }
    }

    if (!preventInputs)
    {
        if (player.x == map->getNCol() - 9 && player.direction == Entity::Direction::RIGHT)
        {
            goToScene = "Road2";
        }

        if (auto event = facedPreviousEvent(player))
        {
            if (event->getId() == "Cave1")
            {
                if (player.direction == Entity::Direction::UP)
                {
                    if (!fadeOutAnimation->isStarted())
                    {
                        fadeOutAnimation->reset();
                        fadeOutAnimation->start();
                        goToScene = "Cave1";
                    }
                }
            }
        }
    }
}

void Road1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);

    if (text1Speech->isStarted() && !text1Speech->shouldClose())
    {
        text1Speech->draw(fps, rs);
    }

    if (childSpeech->isStarted() && !childSpeech->shouldClose())
    {
        childSpeech->draw(fps, rs);
    }
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

        if (event->getId() == "Dive1")
        {
            if (player.direction == Entity::Direction::UP)
            {
                if (!fadeOutAnimation->isStarted())
                {
                    fadeOutAnimation->reset();
                    fadeOutAnimation->start();
                    goToScene = "Dive1";
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
    auto& player = Game::instance()->data.player;

    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (goToScene == "Road2")
    {
        auto scene = std::make_unique<Road2Scene>(renderer);
        scene->initMovingPlayerPosition(9, player.y, 0, Entity::Direction::RIGHT, false);
        return scene;
    }

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

        if (goToScene == "Dive1")
        {
            int  offsetX = player.x - 18;
            int  offsetY = player.y - 18;
            auto scene   = std::make_unique<Dive1Scene>(renderer);
            scene->initPlayerPosition(1 + offsetX, 1 + offsetY, 0, player.direction);
            return scene;
        }
    }
    return nullptr;
}

bool Road1Scene::shouldShowNightTextures() const
{
    return Game::instance()->isNight();
}

std::string Road1Scene::currentLocation() const
{
    auto& player = Game::instance()->data.player;

    if (size_t(player.x) > map->getNCol() / 2)
        return lc::translate("Road 1 right");
    else
        return lc::translate("Road 1 left");
}
