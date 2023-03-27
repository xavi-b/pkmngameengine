#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <vector>
#include <memory>
#include "scene.h"
#include "map.h"

class MapScene : public Scene
{
public:
    MapScene(SDL_Renderer* renderer);

private:
    std::unique_ptr<Map> map;
};

#endif // MAP_H
