#include "road2scene.h"

#include "game.h"
#include "scenes/story/road1scene.h"

Road2Scene::Road2Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/road2.pkmap")
{
}

void Road2Scene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    MapScene::update(inputs);

    if (!preventInputs)
    {
        if (player.x == 8 && player.direction == Entity::Direction::LEFT)
        {
            goToScene = "Road1";
        }
    }
}

std::string Road2Scene::name()
{
    return "Road2Scene";
}

std::unique_ptr<Scene> Road2Scene::nextScene()
{
    auto& player = Game::instance()->data.player;

    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (goToScene == "Road1")
    {
        auto scene = std::make_unique<Road1Scene>(renderer);
        scene->initMovingPlayerPosition(22, player.y, 0, Entity::Direction::LEFT, false);
        return scene;
    }

    return nullptr;
}

bool Road2Scene::shouldShowNightTextures() const
{
    return Game::instance()->isNight();
}

std::string Road2Scene::currentLocation() const
{
    return lc::translate("Road 2");
}
