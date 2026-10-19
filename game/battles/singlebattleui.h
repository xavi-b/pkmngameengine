#ifndef SINGLEBATTLEUI_H
#define SINGLEBATTLEUI_H

#include "fps.h"
#include "pkmn.h"

#include <SDL_image.h>
#include <SDL_mixer.h>

#include <array>
#include <string>

class SingleBattleUi
{
public:
    SingleBattleUi(SDL_Renderer* renderer);
    ~SingleBattleUi();

    void setFoePkmn(Pkmn::PkmnPtr const& newFoePkmn);
    void setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn);

    enum class LineupBall
    {
        Empty,
        Able,
        Status,
        Faint
    };

    // Wild foe slides in with the player's trainer. A trainer foe walks both
    // trainers in first; the opponent throws after the encounter line.
    void startIntro(bool                              foeFromBall,
                    std::string const&                opponentGraphic = {},
                    std::array<LineupBall, 6> const&  lineup          = {});
    bool isFoeEntranceDone() const;
    void startFoeSendOut();
    bool isFoeSendOutStarted() const;
    bool isFoeSendOutDone() const;
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
        TrainerEntrance,
        AwaitTrainers,
        FoeEntrance,
        AwaitSendOut,
        FoeThrow,
        FoeAppear,
        PlayerThrow,
        PlayerAppear,
        Shown,
        SwitchThrow,
        SwitchAppear
    };

    void loadTrainerBack();
    void loadOpponentTrainer(std::string const& path);
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
    SDL_Surface* opponentSurface = nullptr;
    SDL_Texture* opponentTexture = nullptr;
    SDL_Surface* ballSurface    = nullptr;
    SDL_Texture* ballTexture    = nullptr;
    SDL_Surface* burstSurface   = nullptr;
    SDL_Texture* burstTexture   = nullptr;

    SDL_Surface* lineupBarSurface = nullptr;
    SDL_Texture* lineupBarTexture = nullptr;
    std::array<SDL_Surface*, 4> lineupBallSurfaces{};
    std::array<SDL_Texture*, 4> lineupBallTextures{};

    Mix_Chunk* throwChunk = nullptr;
    Mix_Chunk* cryChunk   = nullptr;

    Pkmn::PkmnPtr foePkmn;
    Pkmn::PkmnPtr playerPkmn;

    Stage stage                 = Stage::Shown;
    int   ticks                 = 0;
    bool  introStarted          = false;
    bool  foeSendOutFinished    = false;
    bool  playerSendOutFinished = false;
    bool  showOpponentLineup    = false;

    std::array<LineupBall, 6> opponentLineup{};

    bool   expOverride  = false;
    size_t expShownPrev = 0;
    size_t expShownCur  = 0;
    size_t expSpan      = 1;
};

#endif // SINGLEBATTLEUI_H
