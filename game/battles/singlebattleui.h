#ifndef SINGLEBATTLEUI_H
#define SINGLEBATTLEUI_H

#include "animations/shake/verticalshakeanimation.h"
#include "battleanim.h"
#include "fps.h"
#include "pkmn.h"

#include <SDL_image.h>
#include <SDL_mixer.h>

#include <array>
#include <memory>
#include <string>
#include <vector>

inline char const* battleAnimationStatusScript(Pkmn::StatusCondition status)
{
    switch (status)
    {
    case Pkmn::StatusCondition::BURN:
        return "Status_Burn";
    case Pkmn::StatusCondition::FREEZE:
        return "Status_Freeze";
    case Pkmn::StatusCondition::PARALYSIS:
        return "Status_Paralysis";
    case Pkmn::StatusCondition::POISON:
    case Pkmn::StatusCondition::BADLY_POISON:
        return "Status_Poison";
    case Pkmn::StatusCondition::SLEEP:
        return "Status_Sleep";
    case Pkmn::StatusCondition::NONE:
    default:
        return nullptr;
    }
}

class SingleBattleUi
{
public:
    SingleBattleUi(SDL_Renderer* renderer);
    ~SingleBattleUi();

    void setFoePkmn(Pkmn::PkmnPtr const& newFoePkmn);
    void setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn);
    void syncDisguise(Pkmn::PkmnPtr const& playerShown, Pkmn::PkmnPtr const& foeShown);
    // Reloads battle sprites when Forecast / Zen / Schooling / Ice Face / Hunger Switch change.
    void syncFormeSprites(std::string const& playerSuffix, std::string const& foeSuffix);

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

    void startSwitchSendOut();
    bool isSwitchSendOutDone() const;

    void startFoeSwitchSendOut();

    void incrementTicks();

    // The sprite sinks off its platform. hasFainted stays set until that side is replaced.
    void startFaint(bool playerSide);
    void incrementFaint();
    bool isFaintRunning() const;
    bool hasFainted(bool playerSide) const;

    void setDisplayedExp(size_t shown, size_t span);
    void clearDisplayedExp();

    // Confusion stars and major-status overlays follow the active battlers.
    void setOverlays(bool                    playerConfused,
                     bool                    foeConfused,
                     Pkmn::StatusCondition   playerStatus,
                     Pkmn::StatusCondition   foeStatus);
    void setSemiInvulnerable(bool playerSide, bool foeSide);

    void startMoveAnim(bool                attackerIsPlayer,
                       bool                targetIsPlayer,
                       std::string const&  moveId,
                       std::string const&  moveType,
                       std::string const&  moveName,
                       int                 turn,
                       int                 power,
                       int                 damage);
    void startHit(bool playerSide, std::string const& moveType, std::string const& moveName = {});
    void startStatChange(bool playerSide, bool rose, int palette, bool sharp);
    void startHeal(bool playerSide);
    bool startBattleAnimationScript(bool attackerIsPlayer, bool targetIsPlayer, std::string const& scriptName);
    void startCapture();
    void incrementPresentation();
    bool isPresentationRunning() const;

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
        SwitchAppear,
        FoeSwitchThrow,
        FoeSwitchAppear
    };

    void loadTrainerBack();
    void loadOpponentTrainer(std::string const& path);
    void playThrowSound();
    void playFaintSound();
    void playCry(Pkmn::PkmnPtr const& pkmn);
    int  stageDuration() const;
    float stageProgress(Fps const* fps) const;
    float faintProgress(Fps const* fps) const;

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
    Mix_Chunk* faintChunk = nullptr;
    Mix_Chunk* cryChunk   = nullptr;

    Pkmn::PkmnPtr foePkmn;
    Pkmn::PkmnPtr playerPkmn;
    Pkmn::PkmnPtr foeDisguise;
    Pkmn::PkmnPtr playerDisguise;
    std::string   playerFormeSuffix;
    std::string   foeFormeSuffix;
    void          loadGraphic(Pkmn::PkmnPtr const& look, bool back, SDL_Surface*& surface, SDL_Texture*& texture,
                              std::string const& formeSuffix = {});

    Stage stage                 = Stage::Shown;
    int   ticks                 = 0;
    bool  introStarted          = false;
    bool  foeSendOutFinished    = false;
    bool  playerSendOutFinished = false;
    bool  showOpponentLineup    = false;
    bool  showPlayerLineup      = false;
    bool  foeFainted            = false;
    bool  playerFainted         = false;
    bool  faintRunning          = false;
    bool  faintPlayer           = false;
    int   faintTicks            = 0;

    std::array<LineupBall, 6> opponentLineup{};
    std::array<LineupBall, 6> playerLineup{};

    bool   expOverride  = false;
    size_t expShownPrev = 0;
    size_t expShownCur  = 0;
    size_t expSpan      = 1;

    struct AnimSheet
    {
        SDL_Surface* surface = nullptr;
        SDL_Texture* texture = nullptr;
        std::string  path;
    };

    void      ensureSheet(AnimSheet& sheet, std::string const& path);
    void      freeSheet(AnimSheet& sheet);
    void      drawSheet(AnimSheet const& sheet, SDL_Rect const& anchor, int frameIndex, bool firstRowOnly = false) const;
    int       sheetColumns(AnimSheet const& sheet) const;
    float     shownRatio(float prev, float cur, Fps const* fps) const;
    AnimSheet statusSheetFor(Pkmn::StatusCondition status);

    float foeHpPrev    = 1.f;
    float foeHpCur     = 1.f;
    float playerHpPrev = 1.f;
    float playerHpCur  = 1.f;
    // A damaging move keeps the bar where it was until that animation ends.
    bool deferHpDrop = false;

    bool                  playerConfused        = false;
    bool                  foeConfused           = false;
    bool                  playerSemiInvulnerable = false;
    bool                  foeSemiInvulnerable    = false;
    Pkmn::StatusCondition playerStatus   = Pkmn::StatusCondition::NONE;
    Pkmn::StatusCondition foeStatus      = Pkmn::StatusCondition::NONE;
    int                   overlayTicks   = 0;

    std::unique_ptr<BattleAnimPlayer> moveAnim;
    struct ScriptCue
    {
        std::string name;
        bool        attackerIsPlayer = true;
        bool        targetIsPlayer   = true;
    };
    std::vector<ScriptCue> scriptQueue;
    bool                   overlayBaseline = false;
    Pkmn::StatusCondition  seenPlayerStatus = Pkmn::StatusCondition::NONE;
    Pkmn::StatusCondition  seenFoeStatus    = Pkmn::StatusCondition::NONE;
    bool                   seenPlayerConfused = false;
    bool                   seenFoeConfused    = false;

    bool        hitRunning = false;
    bool        hitPlayer  = false;
    int         hitTicks   = 0;
    int         captureTicks = -1;
    std::string hitType;
    std::unique_ptr<VerticalShakeAnimation> hitShake;

    bool        fxRunning = false;
    bool        fxPlayer  = false;
    int         fxTicks   = 0;
    AnimSheet   fxSheet;

    struct StatCue
    {
        bool playerSide = true;
        bool rose       = true;
        int  palette    = 7;
        bool sharp      = false;
    };
    std::vector<StatCue> statCues;
    bool                 statFxRunning = false;
    bool                 statFxPlayer  = false;
    bool                 statFxRise    = true;
    bool                 statFxSharp   = false;
    int                  statFxPalette = 7;
    Uint32               statFxStart   = 0;
    AnimSheet            statSheet;
    SDL_Surface*         statPattern    = nullptr;
    SDL_Surface*         statMaskSurface = nullptr;
    SDL_Texture*         statMaskTexture = nullptr;
    void                 beginStatFx(bool playerSide, bool rose, int palette, bool sharp);
    void                 advanceStatFx();
    void                 drawStatMask(SDL_Surface* sprite, SDL_Rect const& dst);

    AnimSheet typeSheet;
    AnimSheet confusionSheet;
    AnimSheet burnSheet;
    AnimSheet freezeSheet;
    AnimSheet paralysisSheet;
    AnimSheet poisonSheet;
    AnimSheet sleepSheet;
};

#endif // SINGLEBATTLEUI_H
