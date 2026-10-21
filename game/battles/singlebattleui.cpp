#include "singlebattleui.h"

#include "game.h"
#include "pkmnutils.h"
#include "settings.h"
#include "textspeech.h"
#include "utils.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <map>

namespace
{
class BriefShake final : public VerticalShakeAnimation
{
public:
    explicit BriefShake(SDL_Renderer* renderer) : VerticalShakeAnimation(renderer)
    {
        ticksDuration = 6;
    }
};
} // namespace

namespace
{
constexpr int FoeEntranceTicks = 12;
constexpr int ThrowTicks       = 8;
constexpr int AppearTicks      = 8;

constexpr int BallFrameW     = 32;
constexpr int BallFrameH     = 64;
constexpr int BallFrameCount = 8;
constexpr int AnimCell       = 192;

float clampf(float value, float lo, float hi)
{
    return std::max(lo, std::min(hi, value));
}

SDL_Rect scaledFromFeet(SDL_Rect rect, float scale)
{
    if (scale >= 0.999f)
        return rect;

    SDL_Rect scaled = rect;
    scaled.w        = std::max(1, static_cast<int>(rect.w * scale));
    scaled.h        = std::max(1, static_cast<int>(rect.h * scale));
    scaled.x += (rect.w - scaled.w) / 2;
    scaled.y += rect.h - scaled.h;
    return scaled;
}

// Trainer sheets are 128px with feet at the bottom. Foe fronts are 160px and
// the artwork ends higher, so this lift lines the two foot lines up.
constexpr int OpponentFootOffset    = 26;
constexpr int LineupBarDisplayWidth = 248;
constexpr int LineupBarY            = 114;
constexpr int LineupBallSize        = 30;
constexpr int LineupBallStep        = 32;
constexpr int LineupBallCount       = 6;
constexpr int FaintTicks            = 8;
constexpr int PlayerLineupBarInset  = 142;

SingleBattleUi::LineupBall lineupBallFor(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn)
        return SingleBattleUi::LineupBall::Empty;
    if (pkmn->isKO())
        return SingleBattleUi::LineupBall::Faint;
    if (pkmn->getStatusCondition() != Pkmn::StatusCondition::NONE)
        return SingleBattleUi::LineupBall::Status;
    return SingleBattleUi::LineupBall::Able;
}
} // namespace

SingleBattleUi::SingleBattleUi(SDL_Renderer* renderer)
    : renderer(renderer)
    , moveAnim(std::make_unique<BattleAnimPlayer>(renderer))
{
    playerUiSurface  = IMG_Load("resources/Graphics/UI/Battle/databox_normal.png");
    playerUiTexture  = SDL_CreateTextureFromSurface(renderer, playerUiSurface);
    foeUiSurface     = IMG_Load("resources/Graphics/UI/Battle/databox_normal_foe.png");
    foeUiTexture     = SDL_CreateTextureFromSurface(renderer, foeUiSurface);
    overlayXpSurface = IMG_Load("resources/Graphics/UI/Battle/overlay_exp.png");
    overlayXpTexture = SDL_CreateTextureFromSurface(renderer, overlayXpSurface);
    overlayHpSurface = IMG_Load("resources/Graphics/UI/Battle/overlay_hp.png");
    overlayHpTexture = SDL_CreateTextureFromSurface(renderer, overlayHpSurface);
    statusSurface    = IMG_Load("resources/Graphics/UI/Battle/icon_statuses.png");
    statusTexture    = SDL_CreateTextureFromSurface(renderer, statusSurface);

    lineupBarSurface = IMG_Load("resources/Graphics/UI/Battle/overlay_lineup.png");
    if (lineupBarSurface)
    {
        lineupBarTexture = SDL_CreateTextureFromSurface(renderer, lineupBarSurface);
        if (lineupBarTexture)
            SDL_SetTextureBlendMode(lineupBarTexture, SDL_BLENDMODE_BLEND);
    }

    char const* ballFiles[] = {
        "resources/Graphics/UI/Battle/icon_ball_empty.png",
        "resources/Graphics/UI/Battle/icon_ball.png",
        "resources/Graphics/UI/Battle/icon_ball_status.png",
        "resources/Graphics/UI/Battle/icon_ball_faint.png",
    };
    for (size_t i = 0; i < lineupBallSurfaces.size(); ++i)
    {
        lineupBallSurfaces[i] = IMG_Load(ballFiles[i]);
        if (!lineupBallSurfaces[i])
            continue;
        lineupBallTextures[i] = SDL_CreateTextureFromSurface(renderer, lineupBallSurfaces[i]);
        if (lineupBallTextures[i])
            SDL_SetTextureBlendMode(lineupBallTextures[i], SDL_BLENDMODE_BLEND);
    }

    ballSurface = IMG_Load("resources/Graphics/Battle animations/ball_POKEBALL.png");
    if (ballSurface)
    {
        ballTexture = SDL_CreateTextureFromSurface(renderer, ballSurface);
        if (ballTexture)
            SDL_SetTextureBlendMode(ballTexture, SDL_BLENDMODE_BLEND);
    }

    burstSurface = IMG_Load("resources/Graphics/Battle animations/ballBurst_ring1.png");
    if (burstSurface)
    {
        burstTexture = SDL_CreateTextureFromSurface(renderer, burstSurface);
        if (burstTexture)
            SDL_SetTextureBlendMode(burstTexture, SDL_BLENDMODE_BLEND);
    }

    throwChunk = Mix_LoadWAV("resources/Audio/SE/Battle throw.ogg.wav");
    faintChunk = Mix_LoadWAV("resources/Audio/SE/Pkmn faint.ogg.wav");
}

SingleBattleUi::~SingleBattleUi()
{
    SDL_DestroyTexture(playerUiTexture);
    SDL_FreeSurface(playerUiSurface);
    SDL_DestroyTexture(foeUiTexture);
    SDL_FreeSurface(foeUiSurface);
    SDL_DestroyTexture(overlayXpTexture);
    SDL_FreeSurface(overlayXpSurface);
    SDL_DestroyTexture(overlayHpTexture);
    SDL_FreeSurface(overlayHpSurface);
    SDL_DestroyTexture(statusTexture);
    SDL_FreeSurface(statusSurface);

    if (foeFrontTexture)
        SDL_DestroyTexture(foeFrontTexture);
    if (foeFrontSurface)
        SDL_FreeSurface(foeFrontSurface);

    if (playerBackTexture)
        SDL_DestroyTexture(playerBackTexture);
    if (playerBackSurface)
        SDL_FreeSurface(playerBackSurface);

    if (trainerTexture)
        SDL_DestroyTexture(trainerTexture);
    if (trainerSurface)
        SDL_FreeSurface(trainerSurface);

    if (opponentTexture)
        SDL_DestroyTexture(opponentTexture);
    if (opponentSurface)
        SDL_FreeSurface(opponentSurface);

    if (ballTexture)
        SDL_DestroyTexture(ballTexture);
    if (ballSurface)
        SDL_FreeSurface(ballSurface);

    if (burstTexture)
        SDL_DestroyTexture(burstTexture);
    if (burstSurface)
        SDL_FreeSurface(burstSurface);

    if (lineupBarTexture)
        SDL_DestroyTexture(lineupBarTexture);
    if (lineupBarSurface)
        SDL_FreeSurface(lineupBarSurface);
    for (size_t i = 0; i < lineupBallTextures.size(); ++i)
    {
        if (lineupBallTextures[i])
            SDL_DestroyTexture(lineupBallTextures[i]);
        if (lineupBallSurfaces[i])
            SDL_FreeSurface(lineupBallSurfaces[i]);
    }

    if (throwChunk)
        Mix_FreeChunk(throwChunk);
    if (faintChunk)
        Mix_FreeChunk(faintChunk);
    if (cryChunk)
    {
        Mix_HaltChannel(0);
        Mix_FreeChunk(cryChunk);
    }

    freeSheet(typeSheet);
    freeSheet(fxSheet);
    freeSheet(confusionSheet);
    freeSheet(burnSheet);
    freeSheet(freezeSheet);
    freeSheet(paralysisSheet);
    freeSheet(poisonSheet);
    freeSheet(sleepSheet);
}

void SingleBattleUi::loadTrainerBack()
{
    if (trainerTexture)
        SDL_DestroyTexture(trainerTexture);
    if (trainerSurface)
        SDL_FreeSurface(trainerSurface);
    trainerTexture = nullptr;
    trainerSurface = nullptr;

    std::string file = "resources/Graphics/Trainers/POKEMONTRAINER_";
    file += Game::instance()->data.player.getGender() == Player::Gender::GIRL ? "Leaf_back.png" : "Red_back.png";
    trainerSurface = IMG_Load(file.c_str());
    if (!trainerSurface)
        return;
    trainerTexture = SDL_CreateTextureFromSurface(renderer, trainerSurface);
    if (trainerTexture)
        SDL_SetTextureBlendMode(trainerTexture, SDL_BLENDMODE_BLEND);
}

void SingleBattleUi::loadOpponentTrainer(std::string const& path)
{
    if (opponentTexture)
        SDL_DestroyTexture(opponentTexture);
    if (opponentSurface)
        SDL_FreeSurface(opponentSurface);
    opponentTexture = nullptr;
    opponentSurface = nullptr;

    if (path.empty())
        return;

    opponentSurface = IMG_Load(path.c_str());
    if (!opponentSurface)
        return;
    opponentTexture = SDL_CreateTextureFromSurface(renderer, opponentSurface);
    if (opponentTexture)
        SDL_SetTextureBlendMode(opponentTexture, SDL_BLENDMODE_BLEND);
}

void SingleBattleUi::playThrowSound()
{
    if (!throwChunk || !Settings::instance()->musicOn())
        return;
    Mix_PlayChannel(1, throwChunk, 0);
}

void SingleBattleUi::playFaintSound()
{
    if (!faintChunk || !Settings::instance()->musicOn())
        return;
    Mix_PlayChannel(2, faintChunk, 0);
}

void SingleBattleUi::playCry(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn || !pkmn->getDefinition() || !Settings::instance()->musicOn())
        return;

    if (cryChunk)
    {
        Mix_HaltChannel(0);
        Mix_FreeChunk(cryChunk);
        cryChunk = nullptr;
    }

    std::string path = "resources/Audio/SE/Cries/" + pkmn->getDefinition()->getId() + ".ogg.wav";
    if (!std::filesystem::exists(path))
        return;

    cryChunk = Mix_LoadWAV(path.c_str());
    if (cryChunk)
        Mix_PlayChannel(0, cryChunk, 0);
}

void SingleBattleUi::setFoePkmn(Pkmn::PkmnPtr const& newFoePkmn)
{
    foePkmn          = newFoePkmn;
    foeDisguise.reset();
    foeFormeSuffix.clear();
    foeHpPrev        = foeHpCur = newFoePkmn ? newFoePkmn->getPercentageHP() : 0.f;
    loadGraphic(newFoePkmn, false, foeFrontSurface, foeFrontTexture);
    foeFainted = false;
}

void SingleBattleUi::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn)
{
    playerPkmn       = newPlayerPkmn;
    playerDisguise.reset();
    playerFormeSuffix.clear();
    playerHpPrev     = playerHpCur = newPlayerPkmn ? newPlayerPkmn->getPercentageHP() : 0.f;
    loadGraphic(newPlayerPkmn, true, playerBackSurface, playerBackTexture);
    playerFainted = false;
}

void SingleBattleUi::loadGraphic(Pkmn::PkmnPtr const& look, bool back, SDL_Surface*& surface, SDL_Texture*& texture,
                                 std::string const& formeSuffix)
{
    if (texture)
        SDL_DestroyTexture(texture);
    if (surface)
        SDL_FreeSurface(surface);
    surface = nullptr;
    texture = nullptr;
    if (!look || !look->getDefinition())
        return;

    std::string dir = std::string("resources/Graphics/Pokemon/") + (back ? "Back" : "Front");
    if (look->isShiny())
        dir += " shiny";
    dir += "/";
    std::string stem = look->getDefinition()->getId();
    surface          = IMG_Load((dir + stem + formeSuffix + ".png").c_str());
    if (!surface && !formeSuffix.empty())
        surface = IMG_Load((dir + stem + ".png").c_str());
    if (!surface)
        return;
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture)
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
}

void SingleBattleUi::syncDisguise(Pkmn::PkmnPtr const& playerShown, Pkmn::PkmnPtr const& foeShown)
{
    if (playerShown != playerDisguise)
    {
        playerDisguise = playerShown;
        loadGraphic(playerDisguise ? playerDisguise : playerPkmn, true, playerBackSurface, playerBackTexture,
                    playerFormeSuffix);
    }
    if (foeShown != foeDisguise)
    {
        foeDisguise = foeShown;
        loadGraphic(foeDisguise ? foeDisguise : foePkmn, false, foeFrontSurface, foeFrontTexture, foeFormeSuffix);
    }
}

void SingleBattleUi::syncFormeSprites(std::string const& playerSuffix, std::string const& foeSuffix)
{
    if (playerSuffix != playerFormeSuffix)
    {
        playerFormeSuffix = playerSuffix;
        if (!playerDisguise)
            loadGraphic(playerPkmn, true, playerBackSurface, playerBackTexture, playerFormeSuffix);
    }
    if (foeSuffix != foeFormeSuffix)
    {
        foeFormeSuffix = foeSuffix;
        if (!foeDisguise)
            loadGraphic(foePkmn, false, foeFrontSurface, foeFrontTexture, foeFormeSuffix);
    }
}

void SingleBattleUi::startIntro(bool                             foeFromBall,
                                std::string const&               opponentGraphic,
                                std::array<LineupBall, 6> const& lineup)
{
    introStarted          = true;
    foeSendOutFinished    = !foeFromBall;
    playerSendOutFinished = false;
    showOpponentLineup    = foeFromBall;
    opponentLineup        = lineup;
    showPlayerLineup      = true;
    for (size_t i = 0; i < playerLineup.size(); ++i)
        playerLineup[i] = lineupBallFor(Game::instance()->data.player.pkmns[i]);
    ticks                 = 0;
    loadTrainerBack();

    if (foeFromBall)
    {
        loadOpponentTrainer(opponentGraphic);
        stage = Stage::TrainerEntrance;
    }
    else
    {
        stage = Stage::FoeEntrance;
        playCry(foeDisguise ? foeDisguise : foePkmn);
    }
}

bool SingleBattleUi::isFoeEntranceDone() const
{
    return !introStarted || (stage != Stage::FoeEntrance && stage != Stage::TrainerEntrance);
}

void SingleBattleUi::startFoeSendOut()
{
    if (stage != Stage::AwaitTrainers)
        return;

    stage = Stage::FoeThrow;
    ticks = 0;
    playThrowSound();
}

bool SingleBattleUi::isFoeSendOutStarted() const
{
    return stage == Stage::FoeThrow || stage == Stage::FoeAppear || foeSendOutFinished;
}

bool SingleBattleUi::isFoeSendOutDone() const
{
    return foeSendOutFinished;
}

void SingleBattleUi::startFoeSwitchSendOut()
{
    stage              = Stage::FoeSwitchThrow;
    ticks              = 0;
    foeSendOutFinished = false;
    playThrowSound();
}

void SingleBattleUi::startPlayerSendOut()
{
    if (stage != Stage::AwaitSendOut)
        return;

    stage = Stage::PlayerThrow;
    ticks = 0;
    playThrowSound();
}

bool SingleBattleUi::isPlayerSendOutStarted() const
{
    return stage == Stage::PlayerThrow || stage == Stage::PlayerAppear || playerSendOutFinished;
}

bool SingleBattleUi::isPlayerSendOutDone() const
{
    return playerSendOutFinished;
}

void SingleBattleUi::startSwitchSendOut()
{
    stage                 = Stage::SwitchThrow;
    ticks                 = 0;
    playerSendOutFinished = false;
    playThrowSound();
}

bool SingleBattleUi::isSwitchSendOutDone() const
{
    return stage != Stage::SwitchThrow && stage != Stage::SwitchAppear;
}

void SingleBattleUi::startFaint(bool playerSide)
{
    if (playerSide ? playerFainted : foeFainted)
        return;
    if (faintRunning)
        return;

    faintPlayer  = playerSide;
    faintRunning = true;
    faintTicks   = 0;
    playFaintSound();
}

void SingleBattleUi::incrementFaint()
{
    if (!faintRunning)
        return;

    ++faintTicks;
    if (faintTicks < FaintTicks)
        return;

    faintRunning = false;
    if (faintPlayer)
        playerFainted = true;
    else
        foeFainted = true;
}

bool SingleBattleUi::isFaintRunning() const
{
    return faintRunning;
}

bool SingleBattleUi::hasFainted(bool playerSide) const
{
    return playerSide ? playerFainted : foeFainted;
}

float SingleBattleUi::faintProgress(Fps const* fps) const
{
    if (!faintRunning)
        return 1.f;
    float partial = fps ? fps->tickPercentage() : 0.f;
    return clampf((faintTicks + partial) / static_cast<float>(FaintTicks), 0.f, 1.f);
}

void SingleBattleUi::incrementTicks()
{
    if (stage == Stage::Shown || stage == Stage::AwaitSendOut || stage == Stage::AwaitTrainers)
        return;

    ++ticks;

    int duration = stageDuration();
    if (ticks < duration)
        return;

    ticks = 0;
    switch (stage)
    {
    case Stage::TrainerEntrance:
        stage = Stage::AwaitTrainers;
        break;
    case Stage::FoeEntrance:
        stage = Stage::AwaitSendOut;
        break;
    case Stage::FoeThrow:
        stage = Stage::FoeAppear;
        playCry(foeDisguise ? foeDisguise : foePkmn);
        break;
    case Stage::FoeAppear:
        stage              = Stage::AwaitSendOut;
        foeSendOutFinished = true;
        break;
    case Stage::FoeSwitchThrow:
        stage = Stage::FoeSwitchAppear;
        playCry(foeDisguise ? foeDisguise : foePkmn);
        break;
    case Stage::FoeSwitchAppear:
        stage              = Stage::Shown;
        foeSendOutFinished = true;
        break;
    case Stage::PlayerThrow:
        stage = Stage::PlayerAppear;
        playCry(playerDisguise ? playerDisguise : playerPkmn);
        break;
    case Stage::PlayerAppear:
        stage                 = Stage::Shown;
        playerSendOutFinished = true;
        break;
    case Stage::SwitchThrow:
        stage = Stage::SwitchAppear;
        playCry(playerDisguise ? playerDisguise : playerPkmn);
        break;
    case Stage::SwitchAppear:
        stage                 = Stage::Shown;
        playerSendOutFinished = true;
        break;
    default:
        break;
    }
}

int SingleBattleUi::stageDuration() const
{
    switch (stage)
    {
    case Stage::TrainerEntrance:
        return FoeEntranceTicks;
    case Stage::FoeEntrance:
        return FoeEntranceTicks;
    case Stage::FoeThrow:
    case Stage::PlayerThrow:
    case Stage::SwitchThrow:
    case Stage::FoeSwitchThrow:
        return ThrowTicks;
    case Stage::FoeAppear:
    case Stage::PlayerAppear:
    case Stage::SwitchAppear:
    case Stage::FoeSwitchAppear:
        return AppearTicks;
    default:
        return 1;
    }
}

float SingleBattleUi::stageProgress(Fps const* fps) const
{
    float partial = fps ? fps->tickPercentage() : 0.f;
    return clampf((ticks + partial) / static_cast<float>(stageDuration()), 0.f, 1.f);
}

void SingleBattleUi::setDisplayedExp(size_t shown, size_t span)
{
    if (!expOverride)
        expShownPrev = shown;
    else
        expShownPrev = expShownCur;
    expShownCur = shown;
    expSpan     = span == 0 ? 1 : span;
    expOverride = true;
}

void SingleBattleUi::clearDisplayedExp()
{
    expOverride = false;
}

void SingleBattleUi::freeSheet(AnimSheet& sheet)
{
    if (sheet.texture)
        SDL_DestroyTexture(sheet.texture);
    if (sheet.surface)
        SDL_FreeSurface(sheet.surface);
    sheet.texture = nullptr;
    sheet.surface = nullptr;
    sheet.path.clear();
}

void SingleBattleUi::ensureSheet(AnimSheet& sheet, std::string const& path)
{
    if (sheet.path == path && sheet.texture)
        return;
    freeSheet(sheet);
    sheet.path    = path;
    sheet.surface = IMG_Load(path.c_str());
    if (sheet.surface)
        sheet.texture = SDL_CreateTextureFromSurface(renderer, sheet.surface);
    if (sheet.texture)
        SDL_SetTextureBlendMode(sheet.texture, SDL_BLENDMODE_BLEND);
}

void SingleBattleUi::drawSheet(AnimSheet const& sheet, SDL_Rect const& anchor, int frameIndex, bool firstRowOnly) const
{
    if (!sheet.texture || !sheet.surface || sheet.surface->w <= 0 || sheet.surface->h <= 0)
        return;

    int cols  = std::max(1, sheet.surface->w / AnimCell);
    int rows  = std::max(1, sheet.surface->h / AnimCell);
    int srcW  = std::min(AnimCell, sheet.surface->w);
    int srcH  = std::min(AnimCell, sheet.surface->h);
    int count = std::max(1, firstRowOnly ? cols : cols * rows);
    int frame = frameIndex % count;

    SDL_Rect src;
    src.w = srcW;
    src.h = srcH;
    src.x = (frame % cols) * srcW;
    src.y = firstRowOnly ? 0 : (frame / cols) * srcH;

    SDL_Rect dst;
    dst.w = std::max(1, anchor.w);
    dst.h = std::max(1, anchor.w);
    dst.x = anchor.x + (anchor.w - dst.w) / 2;
    dst.y = anchor.y + anchor.h - dst.h;
    SDL_RenderCopy(renderer, sheet.texture, &src, &dst);
}

int SingleBattleUi::sheetColumns(AnimSheet const& sheet) const
{
    if (!sheet.surface || sheet.surface->w < AnimCell)
        return 1;
    return std::max(1, sheet.surface->w / AnimCell);
}

float SingleBattleUi::shownRatio(float prev, float cur, Fps const* fps) const
{
    float partial = fps ? fps->tickPercentage() : 1.f;
    return clampf(prev + (cur - prev) * partial, 0.f, 1.f);
}

SingleBattleUi::AnimSheet SingleBattleUi::statusSheetFor(Pkmn::StatusCondition status)
{
    switch (status)
    {
    case Pkmn::StatusCondition::BURN:
        ensureSheet(burnSheet, "resources/Graphics/Animations/015-Fire01.png");
        return burnSheet;
    case Pkmn::StatusCondition::FREEZE:
        ensureSheet(freezeSheet, "resources/Graphics/Animations/Ice1.png");
        return freezeSheet;
    case Pkmn::StatusCondition::PARALYSIS:
        ensureSheet(paralysisSheet, "resources/Graphics/Animations/017-Thunder01.png");
        return paralysisSheet;
    case Pkmn::StatusCondition::POISON:
    case Pkmn::StatusCondition::BADLY_POISON:
        ensureSheet(poisonSheet, "resources/Graphics/Animations/poison.png");
        return poisonSheet;
    case Pkmn::StatusCondition::SLEEP:
        ensureSheet(sleepSheet, "resources/Graphics/Animations/028-State01.png");
        return sleepSheet;
    case Pkmn::StatusCondition::NONE:
    default:
        return {};
    }
}

void SingleBattleUi::setOverlays(bool                  playerConfusedIn,
                                 bool                  foeConfusedIn,
                                 Pkmn::StatusCondition playerStatusIn,
                                 Pkmn::StatusCondition foeStatusIn)
{
    playerConfused = playerConfusedIn;
    foeConfused    = foeConfusedIn;
    playerStatus   = playerStatusIn;
    foeStatus      = foeStatusIn;

    if (!overlayBaseline)
    {
        overlayBaseline    = true;
        seenPlayerStatus   = playerStatusIn;
        seenFoeStatus      = foeStatusIn;
        seenPlayerConfused = playerConfusedIn;
        seenFoeConfused    = foeConfusedIn;
        return;
    }

    auto playChange = [this](bool playerSide, Pkmn::StatusCondition previous, Pkmn::StatusCondition current, bool wasConfused,
                             bool confused) {
        if (current != previous)
        {
            if (char const* script = battleAnimationStatusScript(current))
                startBattleAnimationScript(playerSide, playerSide, script);
        }
        if (confused && !wasConfused)
            startBattleAnimationScript(playerSide, playerSide, "Status_Confusion");
    };
    playChange(true, seenPlayerStatus, playerStatusIn, seenPlayerConfused, playerConfusedIn);
    playChange(false, seenFoeStatus, foeStatusIn, seenFoeConfused, foeConfusedIn);
    seenPlayerStatus   = playerStatusIn;
    seenFoeStatus      = foeStatusIn;
    seenPlayerConfused = playerConfusedIn;
    seenFoeConfused    = foeConfusedIn;
}

void SingleBattleUi::setSemiInvulnerable(bool playerSide, bool foeSide)
{
    playerSemiInvulnerable = playerSide;
    foeSemiInvulnerable    = foeSide;
}

namespace
{
std::string normalized(std::string const& text)
{
    std::string out;
    for (unsigned char c : text)
    {
        if (std::isalnum(c))
            out.push_back(static_cast<char>(std::tolower(c)));
    }
    return out;
}

std::string sheetForMove(std::string const& moveName, std::string const& moveType)
{
    static std::map<std::string, std::string> named;
    if (named.empty())
    {
        std::string dir = "resources/Graphics/Animations";
        if (std::filesystem::exists(dir))
        {
            for (auto const& entry : std::filesystem::directory_iterator(dir))
            {
                if (!entry.is_regular_file())
                    continue;
                auto file = entry.path().filename().string();
                if (file.size() < 4 || file.substr(file.size() - 4) != ".png")
                    continue;
                auto stem = file.substr(0, file.size() - 4);
                if (stem.find("Anima") != std::string::npos || stem.find("efftest") != std::string::npos
                    || stem.find("estranho") != std::string::npos || stem.find("zan03") != std::string::npos
                    || stem.find("animsheet") != std::string::npos || stem.find("anim sheet") != std::string::npos)
                    continue;
                named[normalized(stem)] = dir + "/" + file;
            }
        }
    }

    auto found = named.find(normalized(moveName));
    if (found != named.end())
        return found->second;

    if (moveType == "FIRE")
        return "resources/Graphics/Animations/015-Fire01.png";
    if (moveType == "WATER")
        return "resources/Graphics/Animations/018-Water01.png";
    if (moveType == "ELECTRIC")
        return "resources/Graphics/Animations/017-Thunder01.png";
    if (moveType == "ICE")
        return "resources/Graphics/Animations/Ice1.png";
    if (moveType == "DARK")
        return "resources/Graphics/Animations/022-Darkness01.png";
    if (moveType == "GHOST")
        return "resources/Graphics/Animations/ghost1.png";
    if (moveType == "GRASS")
        return "resources/Graphics/Animations/grass.png";
    if (moveType == "POISON")
        return "resources/Graphics/Animations/poison.png";
    if (moveType == "NORMAL")
        return "resources/Graphics/Animations/normal1.png";
    if (moveType == "FIGHTING")
        return "resources/Graphics/Animations/punches.png";
    if (moveType == "GROUND")
        return "resources/Graphics/Animations/Earth1.png";
    if (moveType == "ROCK")
        return "resources/Graphics/Animations/Rock Tomb.png";
    if (moveType == "FLYING")
        return "resources/Graphics/Animations/Wind1.png";
    if (moveType == "PSYCHIC")
        return "resources/Graphics/Animations/Light1.png";
    if (moveType == "DRAGON")
        return "resources/Graphics/Animations/dragon claw.png";
    if (moveType == "STEEL")
        return "resources/Graphics/Animations/Iron Head.png";
    return "resources/Graphics/Animations/030-Explosion01.png";
}
} // namespace

void SingleBattleUi::startMoveAnim(bool               attackerIsPlayer,
                                 bool               targetIsPlayer,
                                 std::string const& moveId,
                                 std::string const& moveType,
                                 std::string const& moveName,
                                 int                turn,
                                 int                power,
                                 int                damage)
{
    deferHpDrop = true;
    if (moveAnim && moveAnim->canPlay(moveId))
    {
        moveAnim->start(moveId, attackerIsPlayer, targetIsPlayer, turn, power, damage);
        return;
    }
    startHit(targetIsPlayer, moveType, moveName);
}

void SingleBattleUi::startHit(bool playerSide, std::string const& moveType, std::string const& moveName)
{
    deferHpDrop = true;
    hitPlayer   = playerSide;
    hitRunning  = true;
    hitTicks   = 0;
    hitType    = moveType;

    ensureSheet(typeSheet, sheetForMove(moveName, moveType));

    if (!hitShake)
        hitShake = std::make_unique<BriefShake>(renderer);
    hitShake->reset();
    hitShake->start();
}

bool SingleBattleUi::startBattleAnimationScript(bool attackerIsPlayer, bool targetIsPlayer, std::string const& scriptName)
{
    if (!moveAnim || !moveAnim->canPlayScript(scriptName))
        return false;
    if (moveAnim->running() || !scriptQueue.empty())
    {
        if (scriptQueue.size() < 8)
            scriptQueue.push_back(ScriptCue{scriptName, attackerIsPlayer, targetIsPlayer});
        return true;
    }
    moveAnim->startScript(scriptName, attackerIsPlayer, targetIsPlayer);
    return moveAnim->running();
}

void SingleBattleUi::startStatRise(bool playerSide)
{
    if (startBattleAnimationScript(playerSide, playerSide, "General_StatsChange"))
        return;
    fxPlayer  = playerSide;
    fxRunning = true;
    fxTicks   = 0;
    ensureSheet(fxSheet, "resources/Graphics/Animations/Growl.png");
}

void SingleBattleUi::startHeal(bool playerSide)
{
    if (startBattleAnimationScript(playerSide, playerSide, "General_WishHeal"))
        return;
    fxPlayer  = playerSide;
    fxRunning = true;
    fxTicks   = 0;
    ensureSheet(fxSheet, "resources/Graphics/Animations/Heal3.png");
}

void SingleBattleUi::startCapture()
{
    captureTicks = 0;
    playThrowSound();
}

void SingleBattleUi::incrementPresentation()
{
    if (moveAnim && moveAnim->running())
        moveAnim->tick();

    if (hitRunning)
    {
        ++hitTicks;
        if (hitShake)
            hitShake->incrementTicks();
        if (hitTicks >= sheetColumns(typeSheet) * 2)
            hitRunning = false;
    }

    // The attack animation has finished. Follow-up scripts can start, and the bar may drop.
    if (deferHpDrop && !hitRunning && !(moveAnim && moveAnim->running()))
        deferHpDrop = false;

    if (moveAnim && !moveAnim->running() && !scriptQueue.empty())
    {
        ScriptCue cue = scriptQueue.front();
        scriptQueue.erase(scriptQueue.begin());
        if (moveAnim->canPlayScript(cue.name))
            moveAnim->startScript(cue.name, cue.attackerIsPlayer, cue.targetIsPlayer);
    }

    ++overlayTicks;
    if (captureTicks >= 0)
    {
        ++captureTicks;
        if (captureTicks >= BallFrameCount)
            captureTicks = -1;
    }
    if (fxRunning)
    {
        ++fxTicks;
        if (fxTicks >= sheetColumns(fxSheet) * 2)
            fxRunning = false;
    }

    auto stepHp = [](float& prev, float& cur, float target) {
        prev      = cur;
        float gap = target - cur;
        if (std::fabs(gap) < 0.008f)
            cur = target;
        else
            cur += gap * 0.4f;
    };
    auto stepBar = [&](float& prev, float& cur, float target) {
        // Damage waits until the move animation is over. Healing can start immediately.
        if (deferHpDrop && target < cur)
            return;
        stepHp(prev, cur, target);
    };

    if (foePkmn)
        stepBar(foeHpPrev, foeHpCur, foePkmn->getPercentageHP());
    if (playerPkmn)
        stepBar(playerHpPrev, playerHpCur, playerPkmn->getPercentageHP());
}

bool SingleBattleUi::isPresentationRunning() const
{
    auto unsettled = [](float shown, float actual) {
        return std::fabs(actual - shown) >= 0.008f;
    };
    bool hpMoving = (foePkmn && unsettled(foeHpCur, foePkmn->getPercentageHP()))
                 || (playerPkmn && unsettled(playerHpCur, playerPkmn->getPercentageHP()));
    return hitRunning || fxRunning || (moveAnim && moveAnim->running()) || hpMoving;
}

void SingleBattleUi::draw(Fps const* fps, RenderSizes rs)
{
    if (!foePkmn || !playerPkmn || !foeFrontSurface || !playerBackSurface || !foeUiSurface || !playerUiSurface
        || !overlayHpSurface || !overlayXpSurface || !statusSurface)
        return;

    int fontSize = RenderUtils::TextSize;

    int bottomMargin    = 2 * TextSpeech::TextBoxHeight;
    int dstBottomMargin = bottomMargin * rs.wh / rs.ah;

    int databoxMargin    = 10;
    int dstDataboxMargin = databoxMargin * rs.wh / rs.ah;

    int textWidth = 0;

    SDL_Color genderColor;
    SDL_Color genderBgColor = {180, 180, 180, 255};

    // https://gamefaqs.gamespot.com/gameboy/367023-pokemon-red-version/faqs/64175/hp-bar-colour
    auto overlayYBasedOnPercentageHP = [](float percentageHP) {
        if (percentageHP > 0.5625)
            return 0;
        else if (percentageHP > 0.207)
            return 1;
        else
            return 2;
    };

    auto statusYBasedOnStatus = [](Pkmn::StatusCondition status) {
        switch (status)
        {
        case Pkmn::StatusCondition::BURN:
            return 2;
        case Pkmn::StatusCondition::FREEZE:
            return 4;
        case Pkmn::StatusCondition::PARALYSIS:
            return 3;
        case Pkmn::StatusCondition::POISON:
            return 1;
        case Pkmn::StatusCondition::BADLY_POISON:
            return 5;
        case Pkmn::StatusCondition::SLEEP:
            return 0;
        default:
            return -1;
        }
    };

    float progress = 1.f;
    if (stage != Stage::Shown && stage != Stage::AwaitSendOut && stage != Stage::AwaitTrainers)
        progress = stageProgress(fps);

    bool drawFoeBox       = true;
    bool drawFoeSprite    = true;
    bool drawPlayerBox    = true;
    bool drawPlayerSprite = true;
    bool drawTrainer      = false;
    bool drawOpponent     = false;
    int  trainerFrame     = 0;
    int  foeBoxDx         = 0;
    int  foeSpriteDx      = 0;
    int  playerBoxDx      = 0;
    int  trainerDx        = 0;
    float opponentSlide   = 0.f;
    float foeScale        = 1.f;
    float playerScale     = 1.f;
    bool  drawBall         = false;
    float ballT            = 0.f;
    bool  ballFromOpponent = false;
    bool  ballFromCorner   = false;
    bool  drawBurst       = false;
    float burstT          = 0.f;
    bool  burstOnFoe      = false;
    bool  drawLineup         = false;
    float lineupShift        = 0.f;
    float lineupAlpha        = 1.f;
    bool  drawPlayerLineup   = false;
    float playerLineupShift  = 0.f;
    float playerLineupAlpha  = 1.f;

    switch (stage)
    {
    case Stage::TrainerEntrance:
        drawFoeSprite    = false;
        drawFoeBox       = false;
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        trainerDx        = static_cast<int>((1.f - progress) * -(160.f * rs.ww / rs.aw));
        drawOpponent     = opponentTexture != nullptr;
        opponentSlide    = 1.f - progress;
        drawLineup        = showOpponentLineup;
        lineupShift       = (1.f - progress) * -(static_cast<float>(rs.aw) / 2.f);
        drawPlayerLineup  = showPlayerLineup;
        playerLineupShift = (1.f - progress) * (static_cast<float>(rs.aw) / 2.f);
        break;
    case Stage::AwaitTrainers:
        drawFoeSprite     = false;
        drawFoeBox        = false;
        drawPlayerSprite  = false;
        drawPlayerBox     = false;
        drawTrainer       = trainerTexture != nullptr;
        drawOpponent      = opponentTexture != nullptr;
        drawLineup        = showOpponentLineup;
        drawPlayerLineup  = showPlayerLineup;
        break;
    case Stage::FoeEntrance: {
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        trainerDx        = static_cast<int>((1.f - progress) * -(160.f * rs.ww / rs.aw));
        int foeTravel    = foeFrontSurface->w * rs.ww / rs.aw * 4 / 3;
        foeSpriteDx      = static_cast<int>((1.f - progress) * foeTravel);
        foeBoxDx         = static_cast<int>((1.f - progress) * -(foeUiSurface->w * rs.ww / rs.aw));
        drawPlayerLineup  = showPlayerLineup;
        playerLineupShift = (1.f - progress) * (static_cast<float>(rs.aw) / 2.f);
        break;
    }
    case Stage::AwaitSendOut:
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        drawPlayerLineup = showPlayerLineup;
        break;
    case Stage::FoeThrow:
        drawFoeSprite    = false;
        drawFoeBox       = false;
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        drawOpponent     = opponentTexture != nullptr;
        drawBall         = true;
        ballFromOpponent = true;
        ballT            = progress;
        drawLineup       = showOpponentLineup;
        drawPlayerLineup = showPlayerLineup;
        break;
    case Stage::FoeAppear:
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        drawOpponent     = opponentTexture != nullptr && progress < 0.85f;
        opponentSlide    = progress;
        foeScale         = 0.25f + 0.75f * progress;
        foeBoxDx         = static_cast<int>((1.f - progress) * -(foeUiSurface->w * rs.ww / rs.aw));
        drawBurst        = progress < 0.75f;
        burstT           = progress;
        burstOnFoe       = true;
        drawLineup        = showOpponentLineup;
        lineupShift       = progress * -(static_cast<float>(rs.aw) / 4.f);
        lineupAlpha       = 1.f - progress;
        drawPlayerLineup  = showPlayerLineup;
        break;
    case Stage::PlayerThrow:
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        trainerFrame     = 3;
        drawBall         = true;
        ballT            = progress;
        drawPlayerLineup = showPlayerLineup;
        break;
    case Stage::PlayerAppear:
        drawTrainer       = false;
        playerScale       = 0.2f + 0.8f * progress;
        playerBoxDx       = static_cast<int>((1.f - progress) * (playerUiSurface->w * rs.ww / rs.aw));
        drawBurst         = progress < 0.7f;
        burstT            = progress;
        drawPlayerLineup  = showPlayerLineup;
        playerLineupShift = progress * (static_cast<float>(rs.aw) / 4.f);
        playerLineupAlpha = 1.f - progress;
        break;
    case Stage::SwitchThrow:
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawBall         = true;
        ballFromCorner   = true;
        ballT            = progress;
        break;
    case Stage::SwitchAppear:
        playerScale = 0.2f + 0.8f * progress;
        playerBoxDx = static_cast<int>((1.f - progress) * (playerUiSurface->w * rs.ww / rs.aw));
        drawBurst   = progress < 0.7f;
        burstT      = progress;
        break;
    case Stage::FoeSwitchThrow:
        drawFoeSprite    = false;
        drawFoeBox       = false;
        drawBall         = true;
        ballFromOpponent = true;
        ballT            = progress;
        break;
    case Stage::FoeSwitchAppear:
        foeScale   = 0.25f + 0.75f * progress;
        foeBoxDx   = static_cast<int>((1.f - progress) * -(foeUiSurface->w * rs.ww / rs.aw));
        drawBurst  = progress < 0.75f;
        burstT     = progress;
        burstOnFoe = true;
        break;
    case Stage::Shown:
        break;
    }

    if (foeFainted || (foeSemiInvulnerable && stage == Stage::Shown))
        drawFoeSprite = false;
    if (playerFainted || (playerSemiInvulnerable && stage == Stage::Shown))
        drawPlayerSprite = false;
    if (moveAnim && moveAnim->running())
    {
        if (moveAnim->battlerHidden(false))
            drawFoeSprite = false;
        if (moveAnim->battlerHidden(true))
            drawPlayerSprite = false;
    }

    if (moveAnim)
    {
        SDL_Rect battleArea;
        battleArea.x = 0;
        battleArea.y = 0;
        battleArea.w = rs.ww;
        battleArea.h = std::max(1, rs.wh - dstBottomMargin);
        moveAnim->drawBackground(battleArea);
    }

    SDL_Rect foeBox;
    foeBox.w = foeUiSurface->w * rs.ww / rs.aw;
    foeBox.h = foeUiSurface->h * rs.wh / rs.ah;
    foeBox.x = foeBoxDx;
    foeBox.y = dstDataboxMargin;

    SDL_Rect playerBox;
    playerBox.w = playerUiSurface->w * rs.ww / rs.aw;
    playerBox.h = playerUiSurface->h * rs.wh / rs.ah;
    playerBox.x = rs.ww - playerBox.w + playerBoxDx;
    playerBox.y = rs.wh - dstBottomMargin - playerBox.h - dstDataboxMargin;

    SDL_Rect foeSprite;
    foeSprite.w = foeFrontSurface->w * rs.ww / rs.aw;
    foeSprite.h = foeFrontSurface->h * rs.wh / rs.ah;
    foeSprite.x = rs.ww - foeSprite.w * 4 / 3 + foeSpriteDx;
    foeSprite.y = dstDataboxMargin * 3;

    SDL_Rect playerSprite;
    playerSprite.w = playerBackSurface->w * rs.ww / rs.aw;
    playerSprite.h = playerBackSurface->h * rs.wh / rs.ah;
    playerSprite.x = playerSprite.w / 3;
    playerSprite.y = rs.wh - playerSprite.h - dstBottomMargin;

    SDL_Rect foeSpriteDrawn    = scaledFromFeet(foeSprite, foeScale);
    SDL_Rect playerSpriteDrawn = scaledFromFeet(playerSprite, playerScale);
    auto     applyMovePose     = [&](SDL_Rect& rect, bool playerSide) {
        if (!moveAnim || !moveAnim->running())
            return;
        float scaleX = moveAnim->monScaleX(playerSide);
        float scaleY = moveAnim->monScaleY(playerSide);
        int   width  = std::max(1, static_cast<int>(rect.w * scaleX));
        int   height = std::max(1, static_cast<int>(rect.h * scaleY));
        rect.x += (rect.w - width) / 2 + moveAnim->offsetX(playerSide, rect.w);
        rect.y += rect.h - height + moveAnim->offsetY(playerSide, rect.h);
        rect.w = width;
        rect.h = height;
    };
    applyMovePose(foeSpriteDrawn, false);
    applyMovePose(playerSpriteDrawn, true);

    int shakeY = (hitRunning && hitShake) ? hitShake->getShakeY(fps, rs) : 0;
    if (hitRunning && !hitPlayer)
        foeSpriteDrawn.y += shakeY;
    if (hitRunning && hitPlayer)
        playerSpriteDrawn.y += shakeY;

    auto drawBattlerFx = [&](bool playerSide, SDL_Rect const& anchor) {
        auto status   = playerSide ? playerStatus : foeStatus;
        bool confused = playerSide ? playerConfused : foeConfused;
        bool scripted = moveAnim && moveAnim->canPlayScript("Status_Confusion");
        if (!scripted && status != Pkmn::StatusCondition::NONE)
            drawSheet(statusSheetFor(status), anchor, overlayTicks / 3);
        if (!scripted && confused)
        {
            ensureSheet(confusionSheet, "resources/Graphics/Animations/confused.png");
            drawSheet(confusionSheet, anchor, overlayTicks / 2);
        }
        if (hitRunning && hitPlayer == playerSide)
            drawSheet(typeSheet, anchor, hitTicks / 2, true);
        if (fxRunning && fxPlayer == playerSide)
            drawSheet(fxSheet, anchor, fxTicks / 2, true);
    };

    SDL_Rect opponentRect{};
    if (opponentSurface)
    {
        opponentRect.w = opponentSurface->w * rs.ww / rs.aw;
        opponentRect.h = opponentSurface->h * rs.wh / rs.ah;
        int settledX   = foeSprite.x + (foeSprite.w - opponentRect.w) / 2;
        int offX       = rs.ww;
        opponentRect.x = settledX + static_cast<int>((offX - settledX) * opponentSlide);
        opponentRect.y = foeSprite.y + foeSprite.h - opponentRect.h - OpponentFootOffset * rs.wh / rs.ah;
    }

    auto drawPartyLineup = [&](std::array<LineupBall, 6> const& lineup,
                               float                            shift,
                               float                            alphaValue,
                               bool                             playerSide) {
        if (!lineupBarTexture || !lineupBarSurface)
            return;

        Uint8 alpha = static_cast<Uint8>(255 * clampf(alphaValue, 0.f, 1.f));
        SDL_SetTextureAlphaMod(lineupBarTexture, alpha);

        int barLogicalX = playerSide ? static_cast<int>(rs.aw - LineupBarDisplayWidth + shift)
                                     : static_cast<int>(shift + LineupBarDisplayWidth - lineupBarSurface->w);
        int barLogicalY = playerSide ? rs.ah - PlayerLineupBarInset : LineupBarY;

        SDL_Rect bar;
        bar.w = lineupBarSurface->w * rs.ww / rs.aw;
        bar.h = lineupBarSurface->h * rs.wh / rs.ah;
        bar.x = barLogicalX * rs.ww / rs.aw;
        bar.y = barLogicalY * rs.wh / rs.ah;
        SDL_RenderCopyEx(renderer,
                         lineupBarTexture,
                         nullptr,
                         &bar,
                         0,
                         nullptr,
                         playerSide ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        SDL_SetTextureAlphaMod(lineupBarTexture, 255);

        for (int i = 0; i < LineupBallCount; ++i)
        {
            auto kind = static_cast<size_t>(lineup[static_cast<size_t>(i)]);
            if (kind >= lineupBallTextures.size() || !lineupBallTextures[kind] || !lineupBallSurfaces[kind])
                continue;

            SDL_SetTextureAlphaMod(lineupBallTextures[kind], alpha);
            SDL_Rect ballIcon;
            ballIcon.w   = lineupBallSurfaces[kind]->w * rs.ww / rs.aw;
            ballIcon.h   = lineupBallSurfaces[kind]->h * rs.wh / rs.ah;
            int logicalX = playerSide ? rs.aw - LineupBarDisplayWidth + 44 + i * LineupBallStep
                                      : LineupBarDisplayWidth - 44 - LineupBallSize - i * LineupBallStep;
            ballIcon.x   = static_cast<int>((logicalX + shift) * rs.ww / rs.aw);
            ballIcon.y   = (barLogicalY - LineupBallSize) * rs.wh / rs.ah;
            SDL_RenderCopy(renderer, lineupBallTextures[kind], nullptr, &ballIcon);
            SDL_SetTextureAlphaMod(lineupBallTextures[kind], 255);
        }
    };

    auto drawBurstAt = [&](SDL_Rect const& anchor) {
        if (!drawBurst || !burstTexture || !burstSurface)
            return;

        float    grow = 0.35f + 1.15f * clampf(burstT, 0.f, 1.f);
        SDL_Rect burst;
        burst.w = static_cast<int>(anchor.w * grow);
        burst.h = static_cast<int>(anchor.h * grow);
        burst.x = anchor.x + anchor.w / 2 - burst.w / 2;
        burst.y = anchor.y + anchor.h / 2 - burst.h / 2;
        Uint8 alpha = static_cast<Uint8>(255 * (1.f - clampf(burstT, 0.f, 1.f)));
        SDL_SetTextureAlphaMod(burstTexture, alpha);
        SDL_RenderCopy(renderer, burstTexture, nullptr, &burst);
        SDL_SetTextureAlphaMod(burstTexture, 255);
    };

    if (drawBurst && burstOnFoe)
        drawBurstAt(foeSprite);

    if (drawFoeSprite && foeFrontTexture)
    {
        if (faintRunning && !faintPlayer)
        {
            float drop = faintProgress(fps);
            foeSpriteDrawn.y += static_cast<int>(foeSpriteDrawn.h * drop);
            SDL_SetTextureAlphaMod(foeFrontTexture, static_cast<Uint8>(255 * (1.f - drop)));
        }
        Uint8 foeRed = 255, foeGreen = 255, foeBlue = 255;
        if (moveAnim)
            moveAnim->battlerColor(false, foeRed, foeGreen, foeBlue);
        if (hitRunning && !hitPlayer)
            foeGreen = foeBlue = 160;
        SDL_SetTextureColorMod(foeFrontTexture, foeRed, foeGreen, foeBlue);
        if (moveAnim && moveAnim->running())
            SDL_SetTextureAlphaMod(foeFrontTexture, moveAnim->battlerAlpha(false));
        SDL_RenderCopy(renderer, foeFrontTexture, nullptr, &foeSpriteDrawn);
        SDL_SetTextureAlphaMod(foeFrontTexture, 255);
        SDL_SetTextureColorMod(foeFrontTexture, 255, 255, 255);
        drawBattlerFx(false, foeSpriteDrawn);
    }

    if (drawOpponent && opponentTexture)
        SDL_RenderCopy(renderer, opponentTexture, nullptr, &opponentRect);

    if (drawTrainer && trainerSurface && trainerTexture)
    {
        int frameSize  = trainerSurface->h > 0 ? trainerSurface->h : trainerSurface->w;
        int frameCount = frameSize > 0 ? trainerSurface->w / frameSize : 1;
        if (frameCount < 1)
            frameCount = 1;
        int frame = trainerFrame;
        if (frame >= frameCount)
            frame = frameCount - 1;

        SDL_Rect src;
        src.x = frame * frameSize;
        src.y = 0;
        src.w = frameSize;
        src.h = trainerSurface->h;

        SDL_Rect dst;
        dst.w = frameSize * rs.ww / rs.aw;
        dst.h = trainerSurface->h * rs.wh / rs.ah;
        dst.x = dst.w / 3 + trainerDx;
        dst.y = rs.wh - dst.h - dstBottomMargin;
        SDL_RenderCopy(renderer, trainerTexture, &src, &dst);
    }

    if (drawBurst && !burstOnFoe)
        drawBurstAt(playerSprite);

    if (drawPlayerSprite && playerBackTexture)
    {
        if (faintRunning && faintPlayer)
        {
            float drop = faintProgress(fps);
            playerSpriteDrawn.y += static_cast<int>(playerSpriteDrawn.h * drop);
            SDL_SetTextureAlphaMod(playerBackTexture, static_cast<Uint8>(255 * (1.f - drop)));
        }
        Uint8 playerRed = 255, playerGreen = 255, playerBlue = 255;
        if (moveAnim)
            moveAnim->battlerColor(true, playerRed, playerGreen, playerBlue);
        if (hitRunning && hitPlayer)
            playerGreen = playerBlue = 160;
        SDL_SetTextureColorMod(playerBackTexture, playerRed, playerGreen, playerBlue);
        if (moveAnim && moveAnim->running())
            SDL_SetTextureAlphaMod(playerBackTexture, moveAnim->battlerAlpha(true));
        SDL_RenderCopy(renderer, playerBackTexture, nullptr, &playerSpriteDrawn);
        SDL_SetTextureAlphaMod(playerBackTexture, 255);
        SDL_SetTextureColorMod(playerBackTexture, 255, 255, 255);
        drawBattlerFx(true, playerSpriteDrawn);
    }

    if (moveAnim)
        moveAnim->drawSprites(playerSpriteDrawn, foeSpriteDrawn);

    if (drawBall && ballTexture && ballSurface)
    {
        int ballW = 40 * rs.ww / rs.aw;
        if (ballW < 1)
            ballW = 1;
        int ballH = ballW * BallFrameH / BallFrameW;

        float startX = 0;
        float startY = 0;
        float endX   = 0;
        float endY   = 0;
        if (ballFromOpponent && opponentSurface)
        {
            startX = static_cast<float>(opponentRect.x + opponentRect.w * 0.35f);
            startY = static_cast<float>(opponentRect.y + opponentRect.h * 0.35f);
            endX   = static_cast<float>(foeSprite.x + foeSprite.w / 2 - ballW / 2);
            endY   = static_cast<float>(foeSprite.y + foeSprite.h / 2 - ballH / 2);
        }
        else if (ballFromOpponent)
        {
            startX = static_cast<float>(rs.ww + ballW);
            startY = static_cast<float>(foeSprite.y + foeSprite.h / 4);
            endX   = static_cast<float>(foeSprite.x + foeSprite.w / 2 - ballW / 2);
            endY   = static_cast<float>(foeSprite.y + foeSprite.h / 2 - ballH / 2);
        }
        else if (ballFromCorner)
        {
            startX = 0;
            startY = static_cast<float>(playerSprite.y + playerSprite.h);
            endX   = static_cast<float>(playerSprite.x + playerSprite.w / 2 - ballW / 2);
            endY   = static_cast<float>(playerSprite.y + playerSprite.h / 3 - ballH / 2);
        }
        else
        {
            startX = static_cast<float>(playerSprite.x + playerSprite.w * 0.15f);
            startY = static_cast<float>(playerSprite.y + playerSprite.h * 0.55f);
            endX   = static_cast<float>(playerSprite.x + playerSprite.w * 0.45f - ballW / 2);
            endY   = static_cast<float>(playerSprite.y + playerSprite.h * 0.35f - ballH / 2);
        }

        float arc = std::sin(clampf(ballT, 0.f, 1.f) * 3.14159265f) * (56.f * rs.wh / rs.ah);
        SDL_Rect ball;
        ball.w = ballW;
        ball.h = ballH;
        ball.x = static_cast<int>(startX + (endX - startX) * ballT);
        ball.y = static_cast<int>(startY + (endY - startY) * ballT - arc);

        int frame = std::clamp(ticks, 0, BallFrameCount - 1);
        SDL_Rect src;
        src.x = frame * BallFrameW;
        src.y = 0;
        src.w = BallFrameW;
        src.h = BallFrameH;
        if (src.x + src.w > ballSurface->w || src.h > ballSurface->h)
        {
            src.x = 0;
            src.y = 0;
            src.w = ballSurface->w;
            src.h = ballSurface->h;
        }
        SDL_RenderCopy(renderer, ballTexture, &src, &ball);
    }

    if (captureTicks >= 0 && captureTicks < BallFrameCount && ballTexture && ballSurface)
    {
        float t = clampf((captureTicks + (fps ? fps->tickPercentage() : 0.f)) / static_cast<float>(BallFrameCount), 0.f, 1.f);
        int ballW = 32 * rs.ww / rs.aw;
        if (ballW < 1)
            ballW = 1;
        int ballH = ballW * BallFrameH / BallFrameW;
        SDL_Rect ball;
        ball.w = ballW;
        ball.h = ballH;
        ball.x = static_cast<int>((1.f - t) * (playerSprite.x + playerSprite.w / 2) + t * (foeSprite.x + foeSprite.w / 2));
        ball.y = static_cast<int>((1.f - t) * (playerSprite.y + playerSprite.h / 3) + t * (foeSprite.y + foeSprite.h / 2));
        SDL_Rect src;
        src.x = captureTicks * BallFrameW;
        src.y = 0;
        src.w = BallFrameW;
        src.h = BallFrameH;
        SDL_RenderCopy(renderer, ballTexture, &src, &ball);
    }

    if (hitRunning)
    {
        int   span  = std::max(1, sheetColumns(typeSheet) * 2);
        float fade  = 1.f - clampf(static_cast<float>(hitTicks) / static_cast<float>(span), 0.f, 1.f);
        Uint8 alpha = static_cast<Uint8>(160.f * fade);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
        SDL_Rect flash;
        flash.x = 0;
        flash.y = 0;
        flash.w = rs.ww;
        flash.h = rs.wh;
        SDL_RenderFillRect(renderer, &flash);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    if (drawLineup)
        drawPartyLineup(opponentLineup, lineupShift, lineupAlpha, false);
    if (drawPlayerLineup)
        drawPartyLineup(playerLineup, playerLineupShift, playerLineupAlpha, true);

    if (drawFoeBox)
    {
        SDL_RenderCopy(renderer, foeUiTexture, nullptr, &foeBox);

        SDL_Rect dstFoeNameRect = foeBox;
        dstFoeNameRect.x += dstDataboxMargin;
        dstFoeNameRect.y += dstDataboxMargin / 2;
        textWidth = RenderUtils::drawGreyText(renderer,
                                              rs,
                                              (foeDisguise ? foeDisguise : foePkmn)->getDisplayName(),
                                              fontSize,
                                              dstFoeNameRect.x,
                                              dstFoeNameRect.y);
        dstFoeNameRect.x += textWidth * rs.ww / rs.aw;
        genderColor = PkmnUtils::genderColor((foeDisguise ? foeDisguise : foePkmn)->getGender());
        RenderUtils::drawUnicodeSymbol(renderer,
                                       rs,
                                       PkmnUtils::genderSymbol((foeDisguise ? foeDisguise : foePkmn)->getGender()),
                                       genderColor,
                                       genderBgColor,
                                       fontSize,
                                       dstFoeNameRect.x,
                                       dstFoeNameRect.y);

        SDL_Rect dstFoeLvlRect = foeBox;
        dstFoeLvlRect.x += foeBox.w - dstDataboxMargin * 4;
        dstFoeLvlRect.y += dstDataboxMargin / 2;
        boost::format foeLvl = boost::format("Lv%1%") % foePkmn->getLevel();
        RenderUtils::drawGreyTextRightAligned(renderer, rs, foeLvl.str(), fontSize, dstFoeLvlRect.x, dstFoeLvlRect.y);

        float    foePercentageHP = shownRatio(foeHpPrev, foeHpCur, fps);
        SDL_Rect dstFoeHpRect    = foeBox;
        dstFoeHpRect.x += 118 * rs.ww / rs.aw;
        dstFoeHpRect.y += 40 * rs.wh / rs.ah;
        dstFoeHpRect.w = foePercentageHP * overlayHpSurface->w * rs.ww / rs.aw;
        dstFoeHpRect.h = overlayHpSurface->h / 3 * rs.wh / rs.ah;
        SDL_Rect srcFoeHpRect;
        srcFoeHpRect.w = overlayHpSurface->w;
        srcFoeHpRect.h = overlayHpSurface->h / 3;
        srcFoeHpRect.x = 0;
        srcFoeHpRect.y = overlayYBasedOnPercentageHP(foePercentageHP) * overlayHpSurface->h / 3;
        SDL_RenderCopy(renderer, overlayHpTexture, &srcFoeHpRect, &dstFoeHpRect);

        int foeStatusY = statusYBasedOnStatus(foePkmn->getStatusCondition());
        if (foeStatusY != -1)
        {
            SDL_Rect dstFoeStatusRect = foeBox;
            dstFoeStatusRect.x += dstDataboxMargin;
            dstFoeStatusRect.y += 35 * rs.wh / rs.ah;
            dstFoeStatusRect.w = statusSurface->w * rs.ww / rs.aw;
            dstFoeStatusRect.h = statusSurface->h * rs.wh / rs.ah / 6;
            SDL_Rect srcFoeStatusRect;
            srcFoeStatusRect.w = statusSurface->w;
            srcFoeStatusRect.h = statusSurface->h / 6;
            srcFoeStatusRect.x = 0;
            srcFoeStatusRect.y = foeStatusY * statusSurface->h / 6;
            SDL_RenderCopy(renderer, statusTexture, &srcFoeStatusRect, &dstFoeStatusRect);
        }
    }

    if (drawPlayerBox)
    {
        SDL_RenderCopy(renderer, playerUiTexture, nullptr, &playerBox);

        SDL_Rect dstPlayerNameRect = playerBox;
        dstPlayerNameRect.x += dstDataboxMargin * 4;
        dstPlayerNameRect.y += dstDataboxMargin / 2;
        textWidth = RenderUtils::drawGreyText(renderer,
                                              rs,
                                              (playerDisguise ? playerDisguise : playerPkmn)->getDisplayName(),
                                              fontSize,
                                              dstPlayerNameRect.x,
                                              dstPlayerNameRect.y);
        dstPlayerNameRect.x += textWidth * rs.ww / rs.aw;
        genderColor = PkmnUtils::genderColor((playerDisguise ? playerDisguise : playerPkmn)->getGender());
        RenderUtils::drawUnicodeSymbol(renderer,
                                       rs,
                                       PkmnUtils::genderSymbol((playerDisguise ? playerDisguise : playerPkmn)->getGender()),
                                       genderColor,
                                       genderBgColor,
                                       fontSize,
                                       dstPlayerNameRect.x,
                                       dstPlayerNameRect.y);

        SDL_Rect dstPlayerLvlRect = playerBox;
        dstPlayerLvlRect.x += playerBox.w - dstDataboxMargin;
        dstPlayerLvlRect.y += dstDataboxMargin / 2;
        boost::format playerLvl = boost::format("Lv%1%") % playerPkmn->getLevel();
        RenderUtils::drawGreyTextRightAligned(renderer,
                                              rs,
                                              playerLvl.str(),
                                              fontSize,
                                              dstPlayerLvlRect.x,
                                              dstPlayerLvlRect.y);

        float    playerPercentageHP = shownRatio(playerHpPrev, playerHpCur, fps);
        SDL_Rect dstPlayerHpRect    = playerBox;
        dstPlayerHpRect.x += 136 * rs.ww / rs.aw;
        dstPlayerHpRect.y += 40 * rs.wh / rs.ah;
        dstPlayerHpRect.w = playerPercentageHP * overlayHpSurface->w * rs.ww / rs.aw;
        dstPlayerHpRect.h = overlayHpSurface->h / 3 * rs.wh / rs.ah;
        SDL_Rect srcPlayerHpRect;
        srcPlayerHpRect.w = overlayHpSurface->w;
        srcPlayerHpRect.h = overlayHpSurface->h / 3;
        srcPlayerHpRect.x = 0;
        srcPlayerHpRect.y = overlayYBasedOnPercentageHP(playerPercentageHP) * overlayHpSurface->h / 3;
        SDL_RenderCopy(renderer, overlayHpTexture, &srcPlayerHpRect, &dstPlayerHpRect);

        int playerStatusY = statusYBasedOnStatus(playerPkmn->getStatusCondition());
        if (playerStatusY != -1)
        {
            SDL_Rect dstPlayerStatusRect = playerBox;
            dstPlayerStatusRect.x += dstDataboxMargin * 4;
            dstPlayerStatusRect.y += 35 * rs.wh / rs.ah;
            dstPlayerStatusRect.w = statusSurface->w * rs.ww / rs.aw;
            dstPlayerStatusRect.h = statusSurface->h * rs.wh / rs.ah / 6;
            SDL_Rect srcPlayerStatusRect;
            srcPlayerStatusRect.w = statusSurface->w;
            srcPlayerStatusRect.h = statusSurface->h / 6;
            srcPlayerStatusRect.x = 0;
            srcPlayerStatusRect.y = playerStatusY * statusSurface->h / 6;
            SDL_RenderCopy(renderer, statusTexture, &srcPlayerStatusRect, &dstPlayerStatusRect);
        }

        float playerPercentageExp = playerPkmn->getPercentageExp();
        if (expOverride)
        {
            float partial = fps ? fps->tickPercentage() : 1.f;
            float shown   = static_cast<float>(expShownPrev)
                        + (static_cast<float>(expShownCur) - static_cast<float>(expShownPrev)) * partial;
            playerPercentageExp = shown / static_cast<float>(expSpan);
        }
        playerPercentageExp = clampf(playerPercentageExp, 0.f, 1.f);

        SDL_Rect dstPlayerXpRect = playerBox;
        dstPlayerXpRect.x += 40 * rs.ww / rs.aw;
        dstPlayerXpRect.y += 74 * rs.wh / rs.ah;
        dstPlayerXpRect.w = playerPercentageExp * overlayXpSurface->w * rs.ww / rs.aw;
        dstPlayerXpRect.h = overlayXpSurface->h * rs.wh / rs.ah;
        SDL_RenderCopy(renderer, overlayXpTexture, nullptr, &dstPlayerXpRect);

        SDL_Rect dstPlayerHpTextRect = playerBox;
        dstPlayerHpTextRect.x += 232 * rs.ww / rs.aw;
        dstPlayerHpTextRect.y += (56 - fontSize / 3) * rs.wh / rs.ah;
        size_t shownPlayerHp = playerPkmn->getHP();
        if (std::fabs(playerPercentageHP - playerPkmn->getPercentageHP()) >= 0.01f)
            shownPlayerHp = static_cast<size_t>(std::lround(playerPercentageHP * playerPkmn->getTotalHP()));
        boost::format playerHp = boost::format("%1% / %2%") % shownPlayerHp % playerPkmn->getTotalHP();
        RenderUtils::drawGreyTextRightAligned(renderer,
                                              rs,
                                              playerHp.str(),
                                              fontSize * 2 / 3,
                                              dstPlayerHpTextRect.x,
                                              dstPlayerHpTextRect.y);
    }
}
