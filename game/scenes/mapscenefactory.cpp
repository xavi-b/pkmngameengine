#include "mapscenefactory.h"

#include "story/cave1scene.h"
#include "story/dive1scene.h"
#include "story/road1scene.h"
#include "story/town1scene.h"

MapSceneFactory::MapSceneFactory(SDL_Renderer* renderer) : renderer(renderer)
{
}

std::unique_ptr<MapScene> MapSceneFactory::fromName(std::string const& name)
{
    if (name == "Town1Scene")
        return std::make_unique<Town1Scene>(renderer);
    if (name == "Road1Scene")
        return std::make_unique<Road1Scene>(renderer);
    if (name == "Cave1Scene")
        return std::make_unique<Cave1Scene>(renderer);
    if (name == "Dive1Scene")
        return std::make_unique<Dive1Scene>(renderer);
    return nullptr;
}
