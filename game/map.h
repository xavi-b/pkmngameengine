#ifndef MAP_H
#define MAP_H

#include <vector>
#include <memory>
#include "scene.h"
#include "layer.h"

class Map : public Scene
{
public:
    Map(SDL_Renderer* renderer);

private:
    std::vector<std::unique_ptr<Layer>> layers;

    // TODO encounters
};

#endif // MAP_H
