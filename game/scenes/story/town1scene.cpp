#include "town1scene.h"

#include "game.h"
#include "scenes/story/house1scene.h"
#include "scenes/story/road1scene.h"
#include "sprites/squaresprite.h"

Town1Scene::Town1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/town1.pkmap")
{
    changeWeather(Map::Weather::RAIN);
}

void Town1Scene::init()
{
    MapScene::init();

    {
        auto entity       = std::make_unique<Entity>();
        npc1Entity        = entity.get();
        entity->x         = 11;
        entity->y         = 10;
        entity->previousX = 11;
        entity->previousY = 10;
        entity->direction = Entity::Direction::RIGHT;
        auto entitySprite = std::make_unique<Sprite>(renderer);
        npc1Sprite        = entitySprite.get();
        entitySprite->load("resources/Graphics/Characters/NPC 01.png", shouldShowNightTextures());
        entities.emplace(std::move(entity), std::move(entitySprite));
    }

    if (!Game::instance()->data.flags["town1_antidote_found"])
    {
        auto entity       = std::make_unique<ErasableEntity>();
        itemEntity        = entity.get();
        entity->x         = 11;
        entity->y         = 2;
        entity->previousX = 11;
        entity->previousY = 2;
        auto itemSprite   = std::make_unique<SquareSprite>(renderer);
        itemSprite->load("resources/Graphics/Characters/Object ball.png", shouldShowNightTextures());
        entities.emplace(std::move(entity), std::move(itemSprite));
    }

    hiddenItemFoundSpeech = std::make_unique<TextSpeech>(renderer);
    hiddenItemFoundSpeech->setTexts({lc::translate("You found a Potion !")});

    itemFoundSpeech = std::make_unique<TextSpeech>(renderer);
    itemFoundSpeech->setTexts({lc::translate("You found an Antidote !")});
}

bool Town1Scene::updateAmbient(Inputs const* inputs)
{
    if (hiddenItemFoundSpeech->isStarted() && !hiddenItemFoundSpeech->shouldClose())
    {
        hiddenItemFoundSpeech->update(inputs);
        return true;
    }

    if (itemFoundSpeech->isStarted() && !itemFoundSpeech->shouldClose())
    {
        itemFoundSpeech->update(inputs);
        return true;
    }

    return false;
}

bool Town1Scene::updateBeforeMovement(Inputs const* /*inputs*/)
{
    auto& player = Game::instance()->data.player;

    auto event = eventAt(player.x, player.y, player.l);

    if (event)
    {
        if (event->getId() == "Road1")
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
                else
                {
                    stopAfterLastMovementFrame(player);
                }
            }
        }
    }

    return false;
}

void Town1Scene::updateAfterMovement(Inputs const* /*inputs*/)
{
    auto& player = Game::instance()->data.player;

    if (npc1Sprite->getAccumulatedTicks() == 0)
    {
        if (npc1Entity->x == 18)
        {
            npc1Entity->direction = Entity::Direction::LEFT;
        }
        else if (npc1Entity->x == 11)
        {
            npc1Entity->direction = Entity::Direction::RIGHT;
        }
        move(*npc1Entity);
    }

    if (auto event = facedPreviousEvent(player))
    {
        if (event->getId() == "House1")
        {
            if (player.direction == Entity::Direction::UP)
            {
                if (!doorOpeningAnimation)
                {
                    doorOpeningAnimation = std::make_unique<DoorAnimation>(renderer, shouldShowNightTextures());
                    doorOpeningAnimation->start();
                    doorOpeningPosition = {player.x, player.y - 1};
                    goToScene           = "House1";
                    player.direction    = Entity::Direction::UP;
                    move(player, true);
                }
            }
        }
    }
}

bool Town1Scene::handleActionButton(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    if (inputs->A)
    {
        if (auto faced = facedEvent(player))
        {
            if (faced->getId() == "HiddenItem1")
            {
                if (Game::instance()->data.flags["town1_potion_found"])
                    return false;

                player.addOneItem(Game::instance()->data.itemDefFor("POTION"));
                hiddenItemFoundSpeech->reset();
                hiddenItemFoundSpeech->start();
                Game::instance()->data.flags["town1_potion_found"] = true;
                return true;
            }
        }
        else if (auto entity = facedEntity(player))
        {
            if (entity == itemEntity && itemEntity != nullptr)
            {
                if (Game::instance()->data.flags["town1_antidote_found"])
                    return false;

                player.addOneItem(Game::instance()->data.itemDefFor("ANTIDOTE"));
                itemEntity->forceErase();
                itemEntity = nullptr;
                itemFoundSpeech->reset();
                itemFoundSpeech->start();
                Game::instance()->data.flags["town1_antidote_found"] = true;
                return true;
            }
        }
    }

    return false;
}

void Town1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);

    if (hiddenItemFoundSpeech->isStarted() && !hiddenItemFoundSpeech->shouldClose())
    {
        hiddenItemFoundSpeech->draw(fps, rs);
    }

    if (itemFoundSpeech->isStarted() && !itemFoundSpeech->shouldClose())
    {
        itemFoundSpeech->draw(fps, rs);
    }
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
            scene->initPlayerPosition(0, 2, 0, Entity::Direction::RIGHT);
            return scene;
        }

        if (goToScene == "House1")
        {
            auto scene = std::make_unique<House1Scene>(renderer);
            scene->initPlayerPosition(1, 7, 0, Entity::Direction::UP);
            return scene;
        }
    }

    return nullptr;
}

bool Town1Scene::shouldShowNightTextures() const
{
    return Game::instance()->isNight();
}

std::string Town1Scene::currentLocation() const
{
    return lc::translate("Town 1");
}
