#include "game.h"

#include "scenes/titlescene.h"

#include <algorithm>

Game::Game(int argc, char* argv[])
    : arguments(argv + 1, argv + argc)
{
    debug = std::find(arguments.begin(), arguments.end(), "--debug") != arguments.end();

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    currentScene = std::make_unique<TitleScene>(renderer);
    currentScene->init();
}

Game::~Game()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

int Game::exec()
{
    fps    = std::make_unique<Fps>(renderer);
    inputs = std::make_unique<Inputs>();

    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
                running = false;
            inputs->update(event);
        }

        if (fps->tick)
        {
            currentScene->sendInputs(inputs.get());
            inputs->clear();
        }

        auto nextScene = currentScene->nextScene();
        if (nextScene)
        {
            currentScene.swap(nextScene);
            currentScene->init();
        }

        SDL_RenderClear(renderer);

        currentScene->draw(fps.get());

        if (debug)
            fps->draw();

        SDL_RenderPresent(renderer);

        SDL_Delay(1);
    }

    return 1;
}
