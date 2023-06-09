#include "town1scene.h"

Town1Scene::Town1Scene(SDL_Renderer* renderer) : MapScene(renderer, "resources/maps/town1.pkmap")
{
}

void Town1Scene::init()
{
    // TODO
    playerX         = 8;
    playerY         = 12;
    playerPreviousX = 8;
    playerPreviousY = 12;
}

void Town1Scene::draw(Fps const* fps, RenderSizes rs)
{
    MapScene::draw(fps, rs);
}

std::string Town1Scene::name()
{
    return "Town1Scene";
}
