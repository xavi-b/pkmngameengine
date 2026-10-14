#include "scene.h"

#include "settings.h"

#include <filesystem>

Scene::Scene(SDL_Renderer* renderer) : renderer(renderer)
{
}

Scene::~Scene()
{
    if (cryChunk)
    {
        Mix_HaltChannel(CryChannel);
        Mix_FreeChunk(cryChunk);
    }
}

Mix_Chunk* Scene::loadChunk(std::string const& path) const
{
    if (path.empty() || !std::filesystem::exists(path))
        return nullptr;
    return Mix_LoadWAV(path.c_str());
}

Mix_Music* Scene::loadMusic(std::string const& path) const
{
    if (path.empty() || !std::filesystem::exists(path))
        return nullptr;
    return Mix_LoadMUS(path.c_str());
}

void Scene::playCry(std::string const& speciesId)
{
    if (cryChunk)
    {
        Mix_HaltChannel(CryChannel);
        Mix_FreeChunk(cryChunk);
        cryChunk = nullptr;
    }
    if (!Settings::instance()->musicOn() || speciesId.empty())
        return;

    cryChunk = loadChunk("resources/Audio/SE/Cries/" + speciesId + ".ogg.wav");
    if (cryChunk)
        Mix_PlayChannel(CryChannel, cryChunk, 0);
}

void Scene::playSound(Mix_Chunk* chunk)
{
    if (!chunk || !Settings::instance()->musicOn())
        return;
    Mix_PlayChannel(SoundChannel, chunk, 0);
}

void Scene::playMusic(Mix_Music* music, int loops)
{
    if (!music || !Settings::instance()->musicOn())
        return;
    Mix_PlayMusic(music, loops);
}

void Scene::stopSounds()
{
    Mix_HaltChannel(-1);
}

void Scene::stopMusic()
{
    Mix_HaltMusic();
}

bool Scene::isCryPlaying() const
{
    if (!Settings::instance()->musicOn() || !cryChunk)
        return false;
    return Mix_Playing(CryChannel) != 0;
}

bool Scene::isMusicPlaying() const
{
    if (!Settings::instance()->musicOn())
        return false;
    return Mix_PlayingMusic() != 0;
}

void Scene::init()
{
}

bool Scene::popScene() const
{
    return false;
}

bool Scene::pushScene() const
{
    return false;
}

void Scene::popReset()
{
}

std::unique_ptr<Scene> Scene::nextScene()
{
    return nullptr;
}

void Scene::debug()
{
}
