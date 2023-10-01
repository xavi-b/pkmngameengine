#ifndef MAPSCENEFACTORY_H
#define MAPSCENEFACTORY_H

#include "mapscene.h"

class MapSceneFactory
{
public:
    MapSceneFactory(SDL_Renderer* renderer);

    std::unique_ptr<MapScene> fromName(std::string const& name);

private:
    SDL_Renderer* renderer;
};

#endif // MAPSCENEFACTORY_H
