#ifndef MAPSCENE_H
#define MAPSCENE_H

#include "map.h"
#include "scene.h"

#include <SDL_image.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class MapScene : public Scene
{
public:
    enum Direction
    {
        NONE,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    enum Speed
    {
        BIKE = 1,
        RUN  = 2,
        WALK = 3
    };

    MapScene(SDL_Renderer* renderer, std::string const& mapPath);
    ~MapScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual void initPlayerPosition(int x, int y, Direction direction);

protected:
    static constexpr int TilePixelSize     = 32;
    static constexpr int PlayerPixelHeight = 48;

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

    Speed speed         = WALK;
    Speed previousSpeed = WALK;
};

#endif // MAP_H
