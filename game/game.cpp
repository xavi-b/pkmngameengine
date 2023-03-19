#include "game.h"

#include "scenes/titlescene.h"
#include "renderutils.h"

#include <algorithm>
#include <iostream>

Game* Game::sInstance = nullptr;

Game::Game(int argc, char* argv[])
    : arguments(argv + 1, argv + argc)
{
    sInstance = this;

    debug = std::find(arguments.begin(), arguments.end(), "--debug") != arguments.end();

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

    window = SDL_CreateWindow(
        PROJECT_NAME,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    scenes.emplace_back(std::make_unique<TitleScene>(renderer));
    scenes.back()->init();
}

Game::~Game()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

Game* Game::instance()
{
    return sInstance;
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
            // TODO show confirmation scene
            if (event.type == SDL_QUIT)
                running = false;
            inputs->update(event);
        }

        if (fps->tick)
        {
            if (inputs->debug)
                printDebug();

            scenes.back()->update(inputs.get());
            inputs->clear();
        }

        if (scenes.back()->popScene())
        {
            scenes.pop_back();
        }
        else
        {
            auto nextScene = scenes.back()->nextScene();
            if (nextScene)
            {
                if (scenes.back()->pushScene())
                    scenes.emplace_back(std::move(nextScene));
                else
                    scenes.back().swap(nextScene);
                scenes.back()->init();
            }
        }

        SDL_RenderClear(renderer);

        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        RenderSizes rs = {w, h, aw, ah};
        scenes.back()->draw(fps.get(), rs);

        if (debug)
            fps->draw(rs);

        SDL_RenderPresent(renderer);

        SDL_Delay(1);
    }

    return 1;
}

void Game::quit()
{
    running = false;
}

void Game::printDebug()
{
    std::cout << "SCENES: ";
    for (const auto& scene : scenes)
        std::cout << scene->name();
    std::cout << std::endl;
}
