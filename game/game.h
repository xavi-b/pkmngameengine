#ifndef GAME_H
#define GAME_H

#include "fps.h"
#include "gamedata.h"
#include "scene.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <boost/locale.hpp>
#include <list>
#include <memory>
#include <string>
#include <vector>

class MapScene;
class MapSceneFactory;

class Game
{
public:
    Game(int argc, char* argv[]);
    ~Game();

    static Game* instance();

    int                       exec();
    void                      quit();
    void                      save();
    std::unique_ptr<MapScene> load();
    void                      printDebug();
    bool                      isDebug();

    GameData data;

private:
    static Game* sInstance;

    constexpr static int aw = 512;
    constexpr static int ah = 384;

    boost::locale::generator gen;

    SDL_Window*   window;
    SDL_Renderer* renderer;

    std::list<std::unique_ptr<Scene>> scenes;
    std::unique_ptr<MapSceneFactory>  mapSceneFactory;

    bool debug   = false;
    bool running = true;

    std::unique_ptr<Fps>    fps;
    std::unique_ptr<Inputs> inputs;
};

#endif // GAME_H
