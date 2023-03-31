#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <SDL_image.h>
#include "scene.h"
#include "map.h"

class MapScene : public Scene
{
public:
    MapScene(SDL_Renderer* renderer, std::string const& mapPath);
    ~MapScene();

    virtual void draw(Fps const* fps, RenderSizes rs) override;

private:
    static constexpr int TilePixelSize = 32;

    std::map<std::string, std::pair<SDL_Surface*, SDL_Texture*>> sprites;

    std::unique_ptr<Map> map;
};

#endif // MAP_H
