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

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

protected:
    static constexpr int TilePixelSize     = 32;
    static constexpr int PlayerPixelHeight = 48;

    enum Direction
    {
        NONE,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    std::map<std::string, std::pair<SDL_Surface*, SDL_Texture*>> sprites;

    SDL_Surface* playerSurface;
    SDL_Texture* playerTexture;

    std::unique_ptr<Map> map;
    size_t               accumulatedTicks = 0;
    Direction            direction        = NONE;
    int                  playerX          = 0;
    int                  playerY          = 0;
    int                  playerPreviousX  = 0;
    int                  playerPreviousY  = 0;
    size_t               playerLevel      = 0;
    size_t               playerSpriteRow  = 0;

    // TODO
    // 4 walk
    // 2 run
    // 1 bike
    int speed = 4;
};

#endif // MAP_H
