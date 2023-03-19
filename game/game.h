#ifndef GAME_H
#define GAME_H

#include <vector>
#include <list>
#include <memory>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <boost/locale.hpp>
#include "scene.h"
#include "fps.h"
#include "gamedata.h"

class Game
{
public:
    Game(int argc, char* argv[]);
    ~Game();

    static Game* instance();

    int  exec();
    void quit();
    void printDebug();

    GameData data;

private:
    static Game* sInstance;

    constexpr static int aw = 512;
    constexpr static int ah = 384;

    boost::locale::generator gen;

    SDL_Window*   window;
    SDL_Renderer* renderer;

    std::list<std::unique_ptr<Scene>> scenes;

    bool debug   = false;
    bool running = true;

    std::unique_ptr<Fps>    fps;
    std::unique_ptr<Inputs> inputs;
};

#endif // GAME_H
