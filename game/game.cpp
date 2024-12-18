#include "game.h"

#include "renderutils.h"
#include "scenes/leavescene.h"
#include "scenes/mapscene.h"
#include "scenes/mapscenefactory.h"
#include "scenes/titlescene.h"
#include "settings.h"

#include <SDL_mixer.h>
#include <algorithm>
#include <boost/program_options.hpp>
#include <ctime>
#include <iostream>

Game* Game::sInstance = nullptr;

namespace po = boost::program_options;

Game::Game(int argc, char* argv[])
{
    sInstance = this;

    po::options_description desc("Allowed options");
    desc.add_options()("help", "help")("locale", po::value<std::string>(), "locale")("debug", "debug")(
        "force-night",
        "force-night")("force-day", "force-day");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        exit(0);
    }
    if (vm.count("debug"))
    {
        debug = true;
    }
    if (vm.count("force-night"))
    {
        forcenight = true;
    }
    if (vm.count("force-day"))
    {
        forceday = true;
    }
    if (vm.count("locale"))
    {
        vm["locale"].as<std::string>();
        std::locale::global(gen("locale"));
    }

    gen.add_messages_path(".");
    gen.add_messages_domain(PROJECT_NAME);

    std::ifstream     dataFile("resources/data.json");
    std::stringstream buffer;
    buffer << dataFile.rdbuf();
    js::value json = js::parse(buffer.str());
    // Types
    data.types = js::value_to<std::vector<Type::TypePtr>>(json.as_object()["types"]);
    // Pkmns
    data.pkmns = js::value_to<std::vector<PkmnDef::PkmnDefPtr>>(json.as_object()["pkmns"]);
    // Moves
    data.moves = js::value_to<std::vector<MoveDef::MoveDefPtr>>(json.as_object()["moves"]);
    // Items
    data.items = js::value_to<std::vector<ItemDef::ItemDefPtr>>(json.as_object()["items"]);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

    window = SDL_CreateWindow(PROJECT_NAME,
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              960,
                              720,
                              SDL_WINDOW_RESIZABLE);

    renderer        = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    mapSceneFactory = std::make_unique<MapSceneFactory>(renderer);

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
        if (fps->tick)
        {
            while (SDL_PollEvent(&event) == 1)
            {
                if (event.type == SDL_QUIT)
                {
                    if (dynamic_cast<LeaveScene*>(scenes.back().get()) == nullptr)
                    {
                        scenes.emplace_back(std::make_unique<LeaveScene>(renderer));
                    }
                }
            }

            inputs->update();

            if (isLoaded())
                data.time.incrementTicks();

            if (inputs->debug)
                printDebug();

            scenes.back()->update(inputs.get());
            inputs->clear();

            if (scenes.back()->popScene())
            {
                printDebug();
                std::cout << scenes.back()->name() << " scene popped !" << std::endl;
                scenes.pop_back();
                scenes.back()->popReset();
                printDebug();
            }
            else
            {
                auto nextScene = scenes.back()->nextScene();
                if (nextScene)
                {
                    printDebug();
                    if (scenes.back()->pushScene())
                    {
                        scenes.emplace_back(std::move(nextScene));
                        std::cout << scenes.back()->name() << " scene pushed !" << std::endl;
                    }
                    else
                    {
                        scenes.back().swap(nextScene);
                        std::cout << scenes.back()->name() << " scene swapped !" << std::endl;
                    }
                    printDebug();
                    scenes.back()->init();
                }
            }
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        RenderSizes rs = {w, h, aw, ah};
        scenes.back()->draw(fps.get(), rs);

        fps->update();

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

void Game::save()
{
    MapScene* currentMapScene = nullptr;
    for (auto it = scenes.rbegin(); it != scenes.rend(); ++it)
    {
        auto mapScene = dynamic_cast<MapScene*>(it->get());
        if (mapScene)
        {
            currentMapScene = mapScene;
            break;
        }
    }

    if (currentMapScene)
    {
        std::chrono::steady_clock::time_point endRealTime = std::chrono::steady_clock::now();
        totalRealTime += std::chrono::duration_cast<std::chrono::seconds>(endRealTime - beginRealTime).count();
        if (Game::instance()->isDebug())
            std::cout << "Total real time playing: " << getFormattedTotalRealTime() << std::endl;
        js::object json;
        json["totalRealTime"] = totalRealTime;
        json["gameMinutes"]   = data.time.getMinutes();
        json["location"]      = data.location;
        json["mapName"]       = currentMapScene->name();
        auto [x, y, l]        = currentMapScene->currentPlayerPosition();
        json["playerX"]       = x;
        json["playerY"]       = y;
        json["playerL"]       = l;
        json["player"]        = js::value_from<Player const&>(data.player);

        std::string dataPath = Utils::dataDir();
        fs::create_directories(dataPath);
        dataPath = dataPath.append("data.json");
        std::ofstream file(dataPath);
        file << js::serialize(json);
        file.close();

        Settings::instance()->setSavedGame(true);

        beginRealTime = std::chrono::steady_clock::now();
    }
}

bool Game::loadData()
{
    if (Settings::instance()->savedGame())
    {
        std::string dataPath = Utils::dataDir();
        fs::create_directories(dataPath);
        dataPath = dataPath.append("data.json");
        std::ifstream     file(dataPath);
        std::stringstream buffer;
        buffer << file.rdbuf();

        if (debug)
            std::cout << buffer.str() << std::endl;

        try
        {
            js::value  json = js::parse(buffer);
            js::object obj  = json.as_object();

            data.player = js::value_to<Player>(obj.at("player"));
            playerX     = js::value_to<size_t>(obj.at("playerX"));
            playerY     = js::value_to<size_t>(obj.at("playerY"));
            playerL     = 0;
            if (obj.contains("playerL"))
                playerL = js::value_to<size_t>(obj.at("playerL"));
            mapSceneName = js::value_to<std::string>(obj.at("mapName"));
            if (obj.contains("totalRealTime"))
                totalRealTime = js::value_to<size_t>(obj.at("totalRealTime"));
            if (obj.contains("gameMinutes"))
                data.time.setMinutes(js::value_to<short>(obj.at("gameMinutes")));
            if (obj.contains("location"))
                data.location = js::value_to<std::string>(obj.at("location"));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    return false;
}

std::unique_ptr<MapScene> Game::loadScene()
{
    loaded        = true;
    beginRealTime = std::chrono::steady_clock::now();
    auto mapScene = mapSceneFactory->fromName(mapSceneName);
    mapScene->initPlayerPosition(playerX, playerY, playerL);
    return mapScene;
}

void Game::printDebug() const
{
    std::cout << "SCENES: ";
    for (auto const& scene : scenes)
        std::cout << scene->name() << " ";
    std::cout << std::endl;
    for (auto const& scene : scenes)
        scene->debug();
}

bool Game::isDebug() const
{
    return debug;
}

bool Game::isDay() const
{
    if (forceday)
        return true;

    return data.time.isDay();
}

bool Game::isNight() const
{
    if (forcenight)
        return true;

    return !isDay();
}

bool Game::isRunning() const
{
    return running;
}

size_t Game::getTotalRealTime() const
{
    return totalRealTime;
}

std::string Game::getFormattedTotalRealTime() const
{
    size_t minutes = totalRealTime / 60;
    return std::to_string((minutes / 60) % 24) + "h" + std::to_string(minutes % 60) + "m";
}

bool Game::isLoaded() const
{
    return loaded;
}
