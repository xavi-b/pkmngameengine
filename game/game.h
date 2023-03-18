#ifndef GAME_H
#define GAME_H

#include <vector>
#include <memory>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "scene.h"
#include "fps.h"

class Game
{
public:
    Game(int argc, char* argv[]);
    ~Game();

    int exec();

private:
    std::vector<std::string> arguments;

    SDL_Window*   window;
    SDL_Renderer* renderer;

    std::unique_ptr<Scene> currentScene;

    bool debug   = false;
    bool running = true;

    std::unique_ptr<Fps>    fps;
    std::unique_ptr<Inputs> inputs;
};

#endif // GAME_H
