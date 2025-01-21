#include "mapscenefactory.h"

#include "scenes/story/cave1scene.h"
#include "scenes/story/cave2scene.h"
#include "scenes/story/cave3scene.h"
#include "scenes/story/dive1scene.h"
#include "scenes/story/house1scene.h"
#include "scenes/story/road1scene.h"
#include "scenes/story/road2scene.h"
#include "scenes/story/sea1scene.h"
#include "scenes/story/town1scene.h"

MapSceneFactory::MapSceneFactory(SDL_Renderer* renderer) : renderer(renderer)
{
}

std::unique_ptr<MapScene> MapSceneFactory::fromName(std::string const& name)
{
    if (name == "Town1Scene")
        return std::make_unique<Town1Scene>(renderer);
    if (name == "Road1Scene")
        return std::make_unique<Road1Scene>(renderer);
    if (name == "House1Scene")
        return std::make_unique<House1Scene>(renderer);
    if (name == "Cave1Scene")
        return std::make_unique<Cave1Scene>(renderer);
    if (name == "Dive1Scene")
        return std::make_unique<Dive1Scene>(renderer);
    if (name == "Road2Scene")
        return std::make_unique<Road2Scene>(renderer);
    if (name == "Sea1Scene")
        return std::make_unique<Sea1Scene>(renderer);
    if (name == "Cave2Scene")
        return std::make_unique<Cave2Scene>(renderer);
    if (name == "Cave3Scene")
        return std::make_unique<Cave3Scene>(renderer);
    return nullptr;
}
