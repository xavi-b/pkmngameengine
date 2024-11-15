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
        entities.emplace(std::move(entity), std::move(entitySprite));
    }

    {
        auto entity       = std::make_unique<Entity>();
        ladyNpc           = entity.get();
        entity->x         = 0;
        entity->y         = 11;
        entity->previousX = 0;
        entity->previousY = 11;
        entity->direction = Entity::Direction::RIGHT;
        auto entitySprite = std::make_unique<Sprite>(renderer);
        entitySprite->load("resources/Graphics/Characters/NPC 04.png", shouldShowNightTextures());
        entities.emplace(std::move(entity), std::move(entitySprite));
    }

    {
        auto entity       = std::make_unique<Entity>();
        entity->x         = 3;
        entity->y         = 12;
        entity->previousX = 3;
        entity->previousY = 12;
        entity->direction = Entity::Direction::NONE;
        auto entitySprite = std::make_unique<Sprite>(renderer);
        entitySprite->load("resources/Graphics/Characters/NPC 06.png", shouldShowNightTextures());
        entitySprite->forceSpriteDirection(Entity::Direction::RIGHT);
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
                    childSpeech->reset();
                    childSpeech->start();
                    preventInputs = true;
                }
            }
        }
    }

    MapScene::update(inputs);

    auto& sprite = *(entities.begin()->second.get());
    if (sprite.getAccumulatedTicks() == 0)
    {
        if (childNpc->x == 10)
        {
            childNpc->direction = Entity::Direction::LEFT;
        }
        else if (childNpc->x == 4)
        {
            childNpc->direction = Entity::Direction::RIGHT;
        }
        move(*childNpc);

        if (ladyNpc->x == 6)
        {
            ladyNpc->direction = Entity::Direction::LEFT;
        }
        else if (ladyNpc->x == 0)
        {
            ladyNpc->direction = Entity::Direction::RIGHT;
        }
        move(*ladyNpc);
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
