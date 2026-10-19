#ifndef SINGLEBATTLEUI_H
#define SINGLEBATTLEUI_H

#include "fps.h"
#include "pkmn.h"

#include <SDL_image.h>
#include <SDL_mixer.h>

class SingleBattleUi
{
public:
    SingleBattleUi(SDL_Renderer* renderer);
    ~SingleBattleUi();

    void setFoePkmn(Pkmn::PkmnPtr const& newFoePkmn);
    void setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn);

    // Wild foe slides in. A trainer foe is thrown in a ball. The player's trainer
    // walks in with them; the player Pokémon is sent out afterwards.
    void startIntro(bool foeFromBall);
    bool isFoeEntranceDone() const;
    void startPlayerSendOut();
    bool isPlayerSendOutStarted() const;
    bool isPlayerSendOutDone() const;

    // Ball throw used when the player switches to another Pokémon.
    void startSwitchSendOut();
    bool isSwitchSendOutDone() const;

    void incrementTicks();

    // While set, the experience bar shows this amount instead of the Pokémon's stored exp.
    void setDisplayedExp(size_t shown, size_t span);
    void clearDisplayedExp();

    void draw(Fps const* fps, RenderSizes rs);

private:
    enum class Stage
    {
        FoeEntrance,
        AwaitSendOut,
        PlayerThrow,
        PlayerAppear,
        Shown,
        SwitchThrow,
        SwitchAppear
    };

    void loadTrainerBack();
    void playThrowSound();
    void playCry(Pkmn::PkmnPtr const& pkmn);
    int  stageDuration() const;
    float stageProgress(Fps const* fps) const;

    SDL_Renderer* renderer;
    SDL_Surface*  playerUiSurface;
    SDL_Texture*  playerUiTexture;
    SDL_Surface*  foeUiSurface;
    SDL_Texture*  foeUiTexture;
    SDL_Surface*  overlayHpSurface;
    SDL_Texture*  overlayHpTexture;
    SDL_Surface*  overlayXpSurface;
    SDL_Texture*  overlayXpTexture;
    SDL_Surface*  statusSurface;
    SDL_Texture*  statusTexture;

    SDL_Surface* playerBackSurface = nullptr;
    SDL_Texture* playerBackTexture = nullptr;
    SDL_Surface* foeFrontSurface   = nullptr;
    SDL_Texture* foeFrontTexture   = nullptr;

    SDL_Surface* trainerSurface = nullptr;
    SDL_Texture* trainerTexture = nullptr;
    SDL_Surface* ballSurface    = nullptr;
    SDL_Texture* ballTexture    = nullptr;
    SDL_Surface* burstSurface   = nullptr;
    SDL_Texture* burstTexture   = nullptr;

    Mix_Chunk* throwChunk = nullptr;
    Mix_Chunk* cryChunk   = nullptr;

    Pkmn::PkmnPtr foePkmn;
    Pkmn::PkmnPtr playerPkmn;

    Stage stage                 = Stage::Shown;
    int   ticks                 = 0;
    bool  introStarted          = false;
    bool  foeFromBall           = false;
    bool  foeCryPlayed          = false;
    bool  playerSendOutFinished = false;

    bool   expOverride  = false;
    size_t expShownPrev = 0;
    size_t expShownCur  = 0;
    size_t expSpan      = 1;
};

#endif // SINGLEBATTLEUI_H
