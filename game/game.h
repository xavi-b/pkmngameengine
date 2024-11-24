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

    GameData data;

    int                       exec();
    void                      quit();
    void                      save();
    bool                      loadData();
    std::unique_ptr<MapScene> loadScene();
    void                      printDebug() const;
    bool                      isDebug() const;
    bool                      isDay() const;
    bool                      isNight() const;
    bool                      isRunning() const;
    size_t                    getTotalRealTime() const;
    std::string               getFormattedTotalRealTime() const;
    bool                      isLoaded() const;

private:
    static Game* sInstance;

    constexpr static int aw = 512;
    constexpr static int ah = 384;

    boost::locale::generator gen;

    SDL_Window*   window;
    SDL_Renderer* renderer;

    std::list<std::unique_ptr<Scene>> scenes;
    std::unique_ptr<MapSceneFactory>  mapSceneFactory;
    std::string                       mapSceneName;
    size_t                            playerX = 0;
    size_t                            playerY = 0;
    size_t                            playerL = 0;

    std::chrono::steady_clock::time_point beginRealTime;
    size_t                                totalRealTime = 0;

    bool debug      = false;
    bool forcenight = false;
    bool forceday   = false;
    bool running    = true;
    bool loaded     = false;

    std::unique_ptr<Fps>    fps;
    std::unique_ptr<Inputs> inputs;
};

#endif // GAME_H
