#ifndef SCENE_H
#define SCENE_H

#include "fps.h"
#include "inputs.h"
#include "renderutils.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <memory>
#include <string>

class Scene
{
public:
    Scene(SDL_Renderer* renderer);
    virtual ~Scene();

    virtual void init();
    virtual void update(Inputs const* inputs)         = 0;
    virtual void draw(Fps const* fps, RenderSizes rs) = 0;

    virtual bool popScene() const;
    virtual bool pushScene() const;
    virtual void popReset();

    virtual std::unique_ptr<Scene> nextScene();

    virtual std::string name() = 0;

    virtual void debug();

    size_t inputId() const
    {
        return inputToken;
    }

protected:
    static constexpr int CryChannel   = 0;
    static constexpr int SoundChannel = 1;

    Mix_Chunk* loadChunk(std::string const& path) const;
    Mix_Music* loadMusic(std::string const& path) const;

    void playCry(std::string const& speciesId);
    void playSound(Mix_Chunk* chunk);
    void playMusic(Mix_Music* music, int loops);
    void stopSounds();
    void stopMusic();

    bool isCryPlaying() const;
    bool isMusicPlaying() const;

    SDL_Renderer* renderer;

private:
    size_t     inputToken = Inputs::newToken();
    Mix_Chunk* cryChunk   = nullptr;
};

#endif // SCENE_H
