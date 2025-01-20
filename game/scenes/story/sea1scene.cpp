#include "sea1scene.h"

#include "game.h"
#include "scenes/story/road2scene.h"

Sea1Scene::Sea1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/sea1.pkmap")
{
}

void Sea1Scene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    MapScene::update(inputs);

    if (!preventInputs)
    {
        if (player.y == 8 && player.direction == Entity::Direction::UP)
        {
            goToScene = "Road2";
        }
    }
}

std::string Sea1Scene::name()
{
    return "Sea1Scene";
}

std::unique_ptr<Scene> Sea1Scene::nextScene()
{
    auto& player = Game::instance()->data.player;

    auto scene = MapScene::nextScene();
    if (scene)
        return scene;

    if (goToScene == "Road2")
    {
        auto scene = std::make_unique<Road2Scene>(renderer);
        scene->initMovingPlayerPosition(player.x + 10, 16, 0, Entity::Direction::UP, false);
        return scene;
    }

    return nullptr;
}

bool Sea1Scene::shouldShowNightTextures() const
{
    return Game::instance()->isNight();
}

std::string Sea1Scene::currentLocation() const
{
    return lc::translate("Sea 1");
}
