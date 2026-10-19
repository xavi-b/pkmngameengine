#include "singlebattleui.h"

#include "game.h"
#include "pkmnutils.h"
#include "settings.h"
#include "textspeech.h"
#include "utils.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

namespace
{
constexpr int FoeEntranceTicks = 12;
constexpr int ThrowTicks       = 8;
constexpr int AppearTicks      = 8;

constexpr int BallFrame = 32;

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
} // namespace

SingleBattleUi::SingleBattleUi(SDL_Renderer* renderer) : renderer(renderer)
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

    if (ballTexture)
        SDL_DestroyTexture(ballTexture);
    if (ballSurface)
        SDL_FreeSurface(ballSurface);

    if (burstTexture)
        SDL_DestroyTexture(burstTexture);
    if (burstSurface)
        SDL_FreeSurface(burstSurface);

    if (throwChunk)
        Mix_FreeChunk(throwChunk);
    if (cryChunk)
    {
        Mix_HaltChannel(0);
        Mix_FreeChunk(cryChunk);
    }
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

void SingleBattleUi::playThrowSound()
{
    if (!throwChunk || !Settings::instance()->musicOn())
        return;
    Mix_PlayChannel(1, throwChunk, 0);
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
    if (foeFrontTexture)
        SDL_DestroyTexture(foeFrontTexture);
    if (foeFrontSurface)
        SDL_FreeSurface(foeFrontSurface);

    foePkmn          = newFoePkmn;
    std::string file = "resources/Graphics/Pokemon/Front";
    if (newFoePkmn->isShiny())
        file += " shiny";
    file += "/";
    file += newFoePkmn->getDefinition()->getId();
    if (newFoePkmn->isFemale())
    {
        std::string femaleFile = file + ".png";
        foeFrontSurface        = IMG_Load(femaleFile.c_str());
        if (foeFrontSurface)
        {
            foeFrontTexture = SDL_CreateTextureFromSurface(renderer, foeFrontSurface);
            return;
        }
    }
    file += ".png";
    foeFrontSurface = IMG_Load(file.c_str());
    foeFrontTexture = SDL_CreateTextureFromSurface(renderer, foeFrontSurface);
}

void SingleBattleUi::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn)
{
    if (playerBackTexture)
        SDL_DestroyTexture(playerBackTexture);
    if (playerBackSurface)
        SDL_FreeSurface(playerBackSurface);

    playerPkmn       = newPlayerPkmn;
    std::string file = "resources/Graphics/Pokemon/Back";
    if (newPlayerPkmn->isShiny())
        file += " shiny";
    file += "/";
    file += newPlayerPkmn->getDefinition()->getId();
    if (newPlayerPkmn->isFemale())
    {
        std::string femaleFile = file + ".png";
        playerBackSurface      = IMG_Load(femaleFile.c_str());
        if (playerBackSurface)
        {
            playerBackTexture = SDL_CreateTextureFromSurface(renderer, playerBackSurface);
            return;
        }
    }
    file += ".png";
    playerBackSurface = IMG_Load(file.c_str());
    playerBackTexture = SDL_CreateTextureFromSurface(renderer, playerBackSurface);
}

void SingleBattleUi::startIntro(bool foeFromBall)
{
    this->foeFromBall     = foeFromBall;
    introStarted          = true;
    foeCryPlayed          = !foeFromBall;
    playerSendOutFinished = false;
    stage                 = Stage::FoeEntrance;
    ticks                 = 0;
    loadTrainerBack();

    if (foeFromBall)
        playThrowSound();
    else
        playCry(foePkmn);
}

bool SingleBattleUi::isFoeEntranceDone() const
{
    return !introStarted || stage != Stage::FoeEntrance;
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
    playerSendOutFinished = true;
    playThrowSound();
}

bool SingleBattleUi::isSwitchSendOutDone() const
{
    return stage != Stage::SwitchThrow && stage != Stage::SwitchAppear;
}

void SingleBattleUi::incrementTicks()
{
    if (stage == Stage::Shown || stage == Stage::AwaitSendOut)
        return;

    ++ticks;

    if (stage == Stage::FoeEntrance && foeFromBall && !foeCryPlayed && ticks >= FoeEntranceTicks / 2)
    {
        playCry(foePkmn);
        foeCryPlayed = true;
    }

    int duration = stageDuration();
    if (ticks < duration)
        return;

    ticks = 0;
    switch (stage)
    {
    case Stage::FoeEntrance:
        stage = Stage::AwaitSendOut;
        break;
    case Stage::PlayerThrow:
        stage = Stage::PlayerAppear;
        playCry(playerPkmn);
        break;
    case Stage::PlayerAppear:
        stage                 = Stage::Shown;
        playerSendOutFinished = true;
        break;
    case Stage::SwitchThrow:
        stage = Stage::SwitchAppear;
        playCry(playerPkmn);
        break;
    case Stage::SwitchAppear:
        stage = Stage::Shown;
        break;
    default:
        break;
    }
}

int SingleBattleUi::stageDuration() const
{
    switch (stage)
    {
    case Stage::FoeEntrance:
        return FoeEntranceTicks;
    case Stage::PlayerThrow:
    case Stage::SwitchThrow:
        return ThrowTicks;
    case Stage::PlayerAppear:
    case Stage::SwitchAppear:
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
    if (stage != Stage::Shown && stage != Stage::AwaitSendOut)
        progress = stageProgress(fps);

    bool drawFoeBox       = true;
    bool drawFoeSprite    = true;
    bool drawPlayerBox    = true;
    bool drawPlayerSprite = true;
    bool drawTrainer      = false;
    int  trainerFrame     = 0;
    int  foeBoxDx         = 0;
    int  foeSpriteDx      = 0;
    int  playerBoxDx      = 0;
    int  trainerDx        = 0;
    float foeScale        = 1.f;
    float playerScale     = 1.f;
    bool  drawBall        = false;
    float ballT           = 0.f;
    bool  ballToFoe       = false;
    bool  ballFromCorner  = false;
    bool  drawBurst       = false;
    float burstT          = 0.f;
    bool  burstOnFoe      = false;

    switch (stage)
    {
    case Stage::FoeEntrance: {
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        trainerDx        = static_cast<int>((1.f - progress) * -(160.f * rs.ww / rs.aw));
        int foeTravel    = foeFrontSurface->w * rs.ww / rs.aw * 4 / 3;
        if (foeFromBall)
        {
            if (progress < 0.55f)
            {
                drawFoeSprite = false;
                drawFoeBox    = false;
                drawBall      = true;
                ballToFoe     = true;
                ballT         = progress / 0.55f;
            }
            else
            {
                float appear = (progress - 0.55f) / 0.45f;
                foeScale     = 0.25f + 0.75f * appear;
                foeBoxDx     = static_cast<int>((1.f - appear) * -(foeUiSurface->w * rs.ww / rs.aw));
                drawBurst    = appear < 0.75f;
                burstT       = appear;
                burstOnFoe   = true;
            }
        }
        else
        {
            foeSpriteDx = static_cast<int>((1.f - progress) * foeTravel);
            foeBoxDx    = static_cast<int>((1.f - progress) * -(foeUiSurface->w * rs.ww / rs.aw));
        }
        break;
    }
    case Stage::AwaitSendOut:
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        break;
    case Stage::PlayerThrow:
        drawPlayerSprite = false;
        drawPlayerBox    = false;
        drawTrainer      = trainerTexture != nullptr;
        trainerFrame     = 3;
        drawBall         = true;
        ballT            = progress;
        break;
    case Stage::PlayerAppear:
        drawTrainer   = false;
        playerScale   = 0.2f + 0.8f * progress;
        playerBoxDx   = static_cast<int>((1.f - progress) * (playerUiSurface->w * rs.ww / rs.aw));
        drawBurst     = progress < 0.7f;
        burstT        = progress;
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
    case Stage::Shown:
        break;
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

    if (drawFoeBox)
    {
        SDL_RenderCopy(renderer, foeUiTexture, nullptr, &foeBox);

        SDL_Rect dstFoeNameRect = foeBox;
        dstFoeNameRect.x += dstDataboxMargin;
        dstFoeNameRect.y += dstDataboxMargin / 2;
        textWidth = RenderUtils::drawGreyText(renderer,
                                              rs,
                                              foePkmn->getDisplayName(),
                                              fontSize,
                                              dstFoeNameRect.x,
                                              dstFoeNameRect.y);
        dstFoeNameRect.x += textWidth * rs.ww / rs.aw;
        genderColor = PkmnUtils::genderColor(foePkmn->getGender());
        RenderUtils::drawUnicodeSymbol(renderer,
                                       rs,
                                       PkmnUtils::genderSymbol(foePkmn->getGender()),
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

        float    foePercentageHP = foePkmn->getPercentageHP();
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
                                              playerPkmn->getDisplayName(),
                                              fontSize,
                                              dstPlayerNameRect.x,
                                              dstPlayerNameRect.y);
        dstPlayerNameRect.x += textWidth * rs.ww / rs.aw;
        genderColor = PkmnUtils::genderColor(playerPkmn->getGender());
        RenderUtils::drawUnicodeSymbol(renderer,
                                       rs,
                                       PkmnUtils::genderSymbol(playerPkmn->getGender()),
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

        float    playerPercentageHP = playerPkmn->getPercentageHP();
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
        boost::format playerHp = boost::format("%1% / %2%") % playerPkmn->getHP() % playerPkmn->getTotalHP();
        RenderUtils::drawGreyTextRightAligned(renderer,
                                              rs,
                                              playerHp.str(),
                                              fontSize * 2 / 3,
                                              dstPlayerHpTextRect.x,
                                              dstPlayerHpTextRect.y);
    }

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
        SDL_RenderCopy(renderer, foeFrontTexture, nullptr, &foeSpriteDrawn);

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
        SDL_RenderCopy(renderer, playerBackTexture, nullptr, &playerSpriteDrawn);

    if (drawBall && ballTexture && ballSurface)
    {
        int ballSize = 40 * rs.ww / rs.aw;
        if (ballSize < 1)
            ballSize = 1;

        float startX = 0;
        float startY = 0;
        float endX   = 0;
        float endY   = 0;
        if (ballToFoe)
        {
            startX = static_cast<float>(rs.ww + ballSize);
            startY = static_cast<float>(foeSprite.y + foeSprite.h / 4);
            endX   = static_cast<float>(foeSprite.x + foeSprite.w / 2 - ballSize / 2);
            endY   = static_cast<float>(foeSprite.y + foeSprite.h / 2 - ballSize / 2);
        }
        else if (ballFromCorner)
        {
            startX = 0;
            startY = static_cast<float>(playerSprite.y + playerSprite.h);
            endX   = static_cast<float>(playerSprite.x + playerSprite.w / 2 - ballSize / 2);
            endY   = static_cast<float>(playerSprite.y + playerSprite.h / 3 - ballSize / 2);
        }
        else
        {
            startX = static_cast<float>(playerSprite.x + playerSprite.w * 0.15f);
            startY = static_cast<float>(playerSprite.y + playerSprite.h * 0.55f);
            endX   = static_cast<float>(playerSprite.x + playerSprite.w * 0.45f - ballSize / 2);
            endY   = static_cast<float>(playerSprite.y + playerSprite.h * 0.35f - ballSize / 2);
        }

        float arc = std::sin(clampf(ballT, 0.f, 1.f) * 3.14159265f) * (56.f * rs.wh / rs.ah);
        SDL_Rect ball;
        ball.w = ballSize;
        ball.h = ballSize;
        ball.x = static_cast<int>(startX + (endX - startX) * ballT);
        ball.y = static_cast<int>(startY + (endY - startY) * ballT - arc);

        SDL_Rect src;
        src.x = 0;
        src.y = 16;
        src.w = BallFrame;
        src.h = BallFrame;
        if (src.y + src.h > ballSurface->h)
        {
            src.y = 0;
            src.h = ballSurface->h;
        }
        SDL_RenderCopy(renderer, ballTexture, &src, &ball);
    }
}
