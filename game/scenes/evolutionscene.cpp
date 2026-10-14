#include "evolutionscene.h"

#include "game.h"
#include "renderutils.h"
#include "utils.h"

#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace
{
constexpr float Pi       = 3.14159265f;
constexpr int   MonX     = 256;
constexpr int   MonY     = 144;
constexpr float ToScreen = 512.f / 240.f;

float gbaCos(int angle, float amplitude)
{
    return std::cos(angle * 2.f * Pi / 256.f) * amplitude;
}

float gbaSin(int angle, float amplitude)
{
    return std::sin(angle * 2.f * Pi / 256.f) * amplitude;
}

SDL_Texture* whiteMask(SDL_Renderer* renderer, SDL_Surface* source)
{
    if (!renderer || !source)
        return nullptr;

    SDL_Surface* surface = SDL_ConvertSurfaceFormat(source, SDL_PIXELFORMAT_RGBA32, 0);
    if (!surface)
        return nullptr;

    SDL_LockSurface(surface);
    for (int y = 0; y < surface->h; ++y)
    {
        auto* row = reinterpret_cast<Uint32*>(static_cast<Uint8*>(surface->pixels) + y * surface->pitch);
        for (int x = 0; x < surface->w; ++x)
        {
            Uint8 r, g, b, a;
            SDL_GetRGBA(row[x], surface->format, &r, &g, &b, &a);
            row[x] = SDL_MapRGBA(surface->format, 255, 255, 255, a);
        }
    }
    SDL_UnlockSurface(surface);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture)
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}
} // namespace

EvolutionScene::EvolutionScene(SDL_Renderer*              renderer,
                               Pkmn::PkmnPtr const&       pkmn,
                               PkmnDef::PkmnDefPtr const& evolutionDef)
    : Scene(renderer), pkmn(pkmn), evolutionDef(evolutionDef)
{
    speech = std::make_unique<TextSpeech>(renderer);

    bgSurface = IMG_Load("resources/Graphics/UI/evolution_bg.png");
    if (bgSurface)
    {
        bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
        if (bgTexture)
            SDL_SetTextureBlendMode(bgTexture, SDL_BLENDMODE_BLEND);
    }

    SDL_Surface* starSurface = IMG_Load("resources/Graphics/Battle animations/ballBurst_star.png");
    if (starSurface)
    {
        starTexture = SDL_CreateTextureFromSurface(renderer, starSurface);
        SDL_FreeSurface(starSurface);
        if (starTexture)
            SDL_SetTextureBlendMode(starTexture, SDL_BLENDMODE_ADD);
    }

    startJingle    = loadMusic("resources/Audio/ME/Evolution start.ogg.wav");
    evolutionMusic = loadMusic("resources/Audio/BGM/Evolution.ogg.wav");
    successMusic   = loadMusic("resources/Audio/ME/Evolution success.ogg.wav");
    spiralSe       = loadChunk("resources/Audio/SE/Anim/Mega Punch.ogg.wav");
    arcSe          = loadChunk("resources/Audio/SE/Anim/Water2.ogg.wav");
    circleSe       = loadChunk("resources/Audio/SE/Anim/Shiny sparkle.ogg.wav");
    spraySe        = loadChunk("resources/Audio/SE/Anim/Twine.ogg.wav");
    poofSe         = loadChunk("resources/Audio/SE/Anim/Flash2.ogg.wav");
}

EvolutionScene::~EvolutionScene()
{
    stopSounds();
    stopMusic();

    destroySprite(preSprite);
    destroySprite(postSprite);

    if (bgTexture)
        SDL_DestroyTexture(bgTexture);
    if (bgSurface)
        SDL_FreeSurface(bgSurface);
    if (starTexture)
        SDL_DestroyTexture(starTexture);

    if (evolutionMusic)
        Mix_FreeMusic(evolutionMusic);
    if (successMusic)
        Mix_FreeMusic(successMusic);
    if (startJingle)
        Mix_FreeMusic(startJingle);
    if (spiralSe)
        Mix_FreeChunk(spiralSe);
    if (arcSe)
        Mix_FreeChunk(arcSe);
    if (circleSe)
        Mix_FreeChunk(circleSe);
    if (spraySe)
        Mix_FreeChunk(spraySe);
    if (poofSe)
        Mix_FreeChunk(poofSe);
}

void EvolutionScene::destroySprite(SpeciesSprite& sprite)
{
    if (sprite.color)
        SDL_DestroyTexture(sprite.color);
    if (sprite.white)
        SDL_DestroyTexture(sprite.white);
    sprite = {};
}

EvolutionScene::SpeciesSprite EvolutionScene::loadSpeciesSprite(std::string const& speciesId) const
{
    SpeciesSprite loaded;
    if (speciesId.empty())
        return loaded;

    std::string folder = "resources/Graphics/Pokemon/Front";
    if (pkmn && pkmn->isShiny())
        folder += " shiny";
    folder += "/";

    std::vector<std::string> candidates;
    if (pkmn && pkmn->isFemale())
        candidates.push_back(folder + speciesId + "_female.png");
    candidates.push_back(folder + speciesId + ".png");

    SDL_Surface* surface = nullptr;
    for (auto const& path : candidates)
    {
        if (!std::filesystem::exists(path))
            continue;
        surface = IMG_Load(path.c_str());
        if (surface)
            break;
    }
    if (!surface)
        return loaded;

    loaded.w     = surface->w;
    loaded.h     = surface->h;
    loaded.color = SDL_CreateTextureFromSurface(renderer, surface);
    loaded.white = whiteMask(renderer, surface);
    SDL_FreeSurface(surface);
    if (loaded.color)
        SDL_SetTextureBlendMode(loaded.color, SDL_BLENDMODE_BLEND);
    return loaded;
}

void EvolutionScene::init()
{
    if (!pkmn || !evolutionDef)
    {
        leave = true;
        return;
    }

    oldName    = pkmn->getDisplayName();
    preSprite  = loadSpeciesSprite(pkmn->getDefinition()->getId());
    postSprite = loadSpeciesSprite(evolutionDef->getId());

    stopMusic();
    enter(Phase::FadeIn);
}

void EvolutionScene::enter(Phase next)
{
    phase      = next;
    phaseFrame = 0;
}

void EvolutionScene::showMessage(std::string const& text)
{
    speech->reset();
    speech->setTexts({text});
    speech->start();
}

void EvolutionScene::commitEvolution()
{
    if (evolved || !pkmn || !evolutionDef)
        return;

    size_t oldMax = pkmn->getTotalHP();
    size_t oldHp  = pkmn->getHP();
    pkmn->setDefinition(evolutionDef);
    size_t newMax = pkmn->getTotalHP();
    if (newMax >= oldMax)
        pkmn->setHP(std::min(newMax, oldHp + (newMax - oldMax)));
    else
        pkmn->setHP(std::min(oldHp, newMax));
    evolved = true;
}

void EvolutionScene::spawnSpiral(int angle)
{
    Sparkle sparkle;
    sparkle.kind   = SparkleKind::SpiralUp;
    sparkle.angle  = angle & 255;
    sparkle.radius = 48;
    sparkles.push_back(sparkle);
}

void EvolutionScene::spawnArc(int angle)
{
    Sparkle sparkle;
    sparkle.kind   = SparkleKind::ArcDown;
    sparkle.angle  = angle & 255;
    sparkle.radius = 8;
    sparkles.push_back(sparkle);
}

void EvolutionScene::spawnCircle(int angle, int shrink)
{
    Sparkle sparkle;
    sparkle.kind   = SparkleKind::CircleIn;
    sparkle.angle  = angle & 255;
    sparkle.radius = 120;
    sparkle.shrink = shrink;
    sparkles.push_back(sparkle);
}

void EvolutionScene::spawnSpray()
{
    Sparkle sparkle;
    sparkle.kind   = SparkleKind::Spray;
    sparkle.radius = static_cast<int>(48 + Utils::randuint(0, 63));
    sparkle.drift  = 3 - static_cast<int>(Utils::randuint(0, 6));
    sparkles.push_back(sparkle);
}

void EvolutionScene::stepSparkles()
{
    for (auto& sparkle : sparkles)
    {
        if (!sparkle.alive)
            continue;

        switch (sparkle.kind)
        {
        case SparkleKind::SpiralUp: {
            int yBase = 88 - sparkle.t * sparkle.t / 80;
            if (yBase <= 8)
                sparkle.alive = false;
            sparkle.angle = (sparkle.angle + 4) & 255;
            if (sparkle.t & 1)
                --sparkle.radius;
            ++sparkle.t;
            break;
        }
        case SparkleKind::ArcDown: {
            int yBase = 8 + sparkle.t * sparkle.t / 5;
            if (yBase >= 88)
                sparkle.alive = false;
            sparkle.radius = static_cast<int>(gbaSin(sparkle.t * 4, 40.f)) + 8;
            ++sparkle.t;
            break;
        }
        case SparkleKind::CircleIn:
            if (sparkle.radius <= 8)
                sparkle.alive = false;
            else
            {
                sparkle.radius -= sparkle.shrink;
                sparkle.angle = (sparkle.angle + 4) & 255;
                ++sparkle.t;
            }
            break;
        case SparkleKind::Spray:
            if (sparkle.angle >= 128)
                sparkle.alive = false;
            else
            {
                ++sparkle.angle;
                ++sparkle.t;
            }
            break;
        }
    }

    sparkles.erase(std::remove_if(sparkles.begin(),
                                  sparkles.end(),
                                  [](Sparkle const& sparkle) {
                                      return !sparkle.alive;
                                  }),
                   sparkles.end());
}

bool EvolutionScene::stepFrame(Inputs const* /*inputs*/)
{
    auto finishPulse = [&]() {
        return (cycleGrowPost && postScale >= 256 && preScale <= 16)
            || (!cycleGrowPost && preScale >= 256 && postScale <= 16);
    };

    switch (phase)
    {
    case Phase::FadeIn:
        fadeAlpha = std::max(0, fadeAlpha - 16);
        if (++phaseFrame >= 16)
        {
            showMessage((boost::format(lc::translate("What? %1% is evolving!")) % oldName).str());
            enter(Phase::IntroMsg);
        }
        return true;
    case Phase::IntroMsg:
        return false;
    case Phase::IntroCry:
        if (++phaseFrame > 4 && !isCryPlaying())
        {
            playMusic(startJingle, 0);
            enter(Phase::IntroJingle);
        }
        return true;
    case Phase::IntroJingle:
        if (++phaseFrame > 8 && !isMusicPlaying())
            enter(Phase::StartMusic);
        return true;
    case Phase::StartMusic:
        if (phaseFrame == 0)
            playMusic(evolutionMusic, -1);
        if (++phaseFrame >= 8)
            enter(Phase::SparkleSpiral);
        return true;
    case Phase::SparkleSpiral:
        if (phaseFrame == 0)
            playSound(spiralSe);
        if (phaseFrame < 64 && (phaseFrame % 8) == 0)
        {
            int base = 2 * (phaseFrame & 0x78);
            for (int i = 0; i < 4; ++i)
                spawnSpiral(base + 64 * i);
        }
        bgAlpha     = std::min(255, phaseFrame * 4);
        spriteWhite = std::min(255, phaseFrame * 4);
        bgAngle     = (bgAngle + 5) & 255;
        stepSparkles();
        if (++phaseFrame >= 160)
            enter(Phase::SparkleArc);
        return true;
    case Phase::SparkleArc:
        if (phaseFrame == 0)
            playSound(arcSe);
        if (phaseFrame < 6)
        {
            for (int i = 0; i < 9; ++i)
                spawnArc(16 * i);
        }
        spriteWhite = 255;
        bgAngle     = (bgAngle + 5) & 255;
        stepSparkles();
        if (++phaseFrame >= 96)
            enter(Phase::CycleSprites);
        return true;
    case Phase::CycleSprites:
        if (phaseFrame == 0)
        {
            preScale      = 256;
            postScale     = 16;
            cycleGrowPost = false;
            cyclePulsing  = false;
            cycleSpeed    = 8;
            spriteWhite   = 255;
            showPost      = false;
        }
        bgAngle = (bgAngle + 6) & 255;
        stepSparkles();
        if (!cyclePulsing)
        {
            if (cycleSpeed >= 128)
            {
                showPost    = true;
                spriteWhite = 0;
                preScale    = 16;
                postScale   = 256;
                enter(Phase::SparkleCircle);
                return true;
            }
            cycleSpeed += 2;
            cycleGrowPost = !cycleGrowPost;
            cyclePulsing  = true;
        }
        else
        {
            int preTarget  = cycleGrowPost ? 16 : 256;
            int postTarget = cycleGrowPost ? 256 : 16;
            if (preScale < preTarget)
                preScale = std::min(preTarget, preScale + cycleSpeed);
            else if (preScale > preTarget)
                preScale = std::max(preTarget, preScale - cycleSpeed);
            if (postScale < postTarget)
                postScale = std::min(postTarget, postScale + cycleSpeed);
            else if (postScale > postTarget)
                postScale = std::max(postTarget, postScale - cycleSpeed);
            if (finishPulse())
                cyclePulsing = false;
        }
        ++phaseFrame;
        return true;
    case Phase::SparkleCircle:
        if (phaseFrame == 0)
        {
            playSound(circleSe);
            for (int i = 0; i < 16; ++i)
                spawnCircle(i * 16, 4);
        }
        if (phaseFrame == 32)
        {
            for (int i = 0; i < 16; ++i)
                spawnCircle(i * 16, 8);
        }
        showPost = true;
        bgAngle  = (bgAngle + 6) & 255;
        stepSparkles();
        if (++phaseFrame >= 48)
            enter(Phase::SparkleSpray);
        return true;
    case Phase::SparkleSpray:
        if (phaseFrame == 0)
        {
            playSound(spraySe);
            for (int i = 0; i < 8; ++i)
                spawnSpray();
            flashAlpha = 0;
        }
        else if (phaseFrame < 50)
            spawnSpray();
        if (phaseFrame < 16)
            flashAlpha = std::min(255, flashAlpha + 16);
        else if (phaseFrame >= 32)
            flashAlpha = std::max(0, flashAlpha - 8);
        bgAngle = (bgAngle + 7) & 255;
        stepSparkles();
        if (++phaseFrame >= 128)
            enter(Phase::EvoSound);
        return true;
    case Phase::EvoSound:
        if (phaseFrame == 0)
            playSound(poofSe);
        if (++phaseFrame >= 24)
            enter(Phase::RestoreScreen);
        return true;
    case Phase::RestoreScreen:
        bgAlpha = std::max(0, bgAlpha - 16);
        stepSparkles();
        if (++phaseFrame >= 20)
        {
            stopMusic();
            sparkles.clear();
            playCry(evolutionDef->getId());
            enter(Phase::EvoCry);
        }
        return true;
    case Phase::EvoCry:
        if (++phaseFrame > 4 && !isCryPlaying())
        {
            commitEvolution();
            std::string evolvedName = evolutionDef->getId();
            showMessage(
                (boost::format(lc::translate("Congratulations! Your %1% evolved into %2%!")) % oldName % evolvedName)
                    .str());
            playMusic(successMusic, 0);
            enter(Phase::CongratsMsg);
        }
        return true;
    case Phase::CongratsMsg:
        return false;
    case Phase::CancelFlash:
        stopMusic();
        flashAlpha  = std::max(0, 255 - phaseFrame * 20);
        showPost    = false;
        spriteWhite = 0;
        bgAlpha     = std::max(0, bgAlpha - 32);
        if (++phaseFrame >= 14)
        {
            playCry(pkmn->getDefinition()->getId());
            enter(Phase::CancelCry);
        }
        return true;
    case Phase::CancelCry:
        if (++phaseFrame > 4 && !isCryPlaying())
        {
            showMessage((boost::format(lc::translate("Huh? %1% stopped evolving!")) % oldName).str());
            enter(Phase::CancelMsg);
        }
        return true;
    case Phase::CancelMsg:
        return false;
    case Phase::FadeOut:
        fadeAlpha = std::min(255, fadeAlpha + 16);
        if (++phaseFrame >= 16)
            leave = true;
        return true;
    }
    return false;
}

void EvolutionScene::update(Inputs const* inputs)
{
    if (leave)
        return;

    Uint32 now = SDL_GetTicks();
    if (clockMs == 0)
        clockMs = now;
    msAccum += static_cast<int>(now - clockMs);
    clockMs = now;
    if (msAccum > 100)
        msAccum = 100;

    if (phase == Phase::IntroMsg || phase == Phase::CongratsMsg || phase == Phase::CancelMsg)
    {
        inputs->setHandler(speech->inputId());
        speech->update(inputs);
        bool introReady = phase == Phase::IntroMsg && speech->mayClose() && (inputs->A || ++phaseFrame >= 6);
        if (speech->shouldClose() || introReady)
        {
            if (phase == Phase::IntroMsg)
            {
                playCry(pkmn->getDefinition()->getId());
                enter(Phase::IntroCry);
            }
            else
            {
                fadeAlpha = 0;
                enter(Phase::FadeOut);
            }
        }
        return;
    }

    inputs->setHandler(inputId());

    if (phase == Phase::CycleSprites && inputs->B)
    {
        sparkles.clear();
        flashAlpha = 255;
        enter(Phase::CancelFlash);
    }

    while (msAccum >= 16)
    {
        msAccum -= 16;
        if (!stepFrame(inputs))
            break;
    }
}

bool EvolutionScene::sparklePosition(Sparkle const& sparkle, float& x, float& y, float& scale, int& alpha) const
{
    if (!sparkle.alive)
        return false;

    float relX = 0;
    float relY = 0;
    scale      = 1;
    alpha      = 255;

    switch (sparkle.kind)
    {
    case SparkleKind::SpiralUp: {
        int yBase = 88 - sparkle.t * sparkle.t / 80;
        relX      = gbaCos(sparkle.angle, static_cast<float>(sparkle.radius));
        relY      = (yBase - 88) + gbaSin(sparkle.angle, static_cast<float>(sparkle.radius)) / 4.f;
        scale     = std::clamp(sparkle.radius / 40.f, 0.25f, 1.3f);
        break;
    }
    case SparkleKind::ArcDown: {
        int yBase = 8 + sparkle.t * sparkle.t / 5;
        relX      = gbaCos(sparkle.angle, static_cast<float>(std::max(0, sparkle.radius)));
        relY      = (yBase - 88) + gbaSin(sparkle.angle, static_cast<float>(std::max(0, sparkle.radius))) / 4.f;
        scale     = 0.7f;
        break;
    }
    case SparkleKind::CircleIn:
        relX  = gbaCos(sparkle.angle, static_cast<float>(sparkle.radius));
        relY  = gbaSin(sparkle.angle, static_cast<float>(sparkle.radius));
        scale = std::clamp(sparkle.radius / 90.f, 0.2f, 1.4f);
        break;
    case SparkleKind::Spray:
        relX  = static_cast<float>(sparkle.drift * sparkle.t) / 3.f;
        relY  = -gbaSin(sparkle.angle, static_cast<float>(sparkle.radius)) + sparkle.t / 4.f;
        scale = std::clamp(1.3f - sparkle.angle / 110.f, 0.2f, 1.3f);
        if (sparkle.angle > 112 && (sparkle.angle & 1))
            alpha = 0;
        break;
    }

    x = MonX + relX * ToScreen;
    y = MonY + relY * ToScreen;
    return alpha > 0;
}

void EvolutionScene::drawBackground(RenderSizes rs)
{
    if (!bgTexture || bgAlpha <= 0)
        return;

    int wobbleX = static_cast<int>(gbaCos(bgAngle, 10.f));
    int wobbleY = static_cast<int>(gbaSin(bgAngle, 10.f));
    int pulse   = static_cast<int>(gbaSin(bgAngle * 3, 50.f)) + 180;

    SDL_SetTextureAlphaMod(bgTexture, static_cast<Uint8>(std::clamp(bgAlpha, 0, 255)));
    SDL_SetTextureColorMod(bgTexture, static_cast<Uint8>(std::clamp(pulse, 140, 255)), 255, 230);

    SDL_Rect dst;
    dst.w = rs.ww + 24 * rs.ww / rs.aw;
    dst.h = rs.wh + 24 * rs.wh / rs.ah;
    dst.x = wobbleX * rs.ww / rs.aw - 12 * rs.ww / rs.aw;
    dst.y = wobbleY * rs.wh / rs.ah - 12 * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, bgTexture, nullptr, &dst);
}

void EvolutionScene::drawMon(RenderSizes rs)
{
    auto blit = [&](SDL_Texture* texture, int srcW, int srcH, float scale) {
        if (!texture || srcW <= 0 || srcH <= 0 || scale <= 0.02f)
            return;
        SDL_Rect dst;
        dst.w = static_cast<int>(srcW * scale * rs.ww / rs.aw);
        dst.h = static_cast<int>(srcH * scale * rs.wh / rs.ah);
        dst.x = MonX * rs.ww / rs.aw - dst.w / 2;
        dst.y = MonY * rs.wh / rs.ah - dst.h / 2;
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
    };

    bool cycling = phase == Phase::CycleSprites;
    if (cycling)
    {
        float pre  = preScale / 256.f;
        float post = postScale / 256.f;
        if (pre >= post)
        {
            blit(preSprite.white, preSprite.w, preSprite.h, pre);
            blit(postSprite.white, postSprite.w, postSprite.h, post);
        }
        else
        {
            blit(postSprite.white, postSprite.w, postSprite.h, post);
            blit(preSprite.white, preSprite.w, preSprite.h, pre);
        }
        return;
    }

    SpeciesSprite const& sprite = showPost ? postSprite : preSprite;
    blit(sprite.color, sprite.w, sprite.h, 1.f);
    if (spriteWhite > 0 && sprite.white)
    {
        SDL_SetTextureAlphaMod(sprite.white, static_cast<Uint8>(std::clamp(spriteWhite, 0, 255)));
        blit(sprite.white, sprite.w, sprite.h, 1.f);
    }
}

void EvolutionScene::drawSparkles(RenderSizes rs)
{
    if (!starTexture)
        return;

    for (auto const& sparkle : sparkles)
    {
        float x, y, scale;
        int   alpha = 255;
        if (!sparklePosition(sparkle, x, y, scale, alpha))
            continue;

        int size = static_cast<int>(22.f * scale);
        if (size < 2)
            continue;

        SDL_Rect dst;
        dst.w = size * rs.ww / rs.aw;
        dst.h = size * rs.wh / rs.ah;
        dst.x = static_cast<int>(x) * rs.ww / rs.aw - dst.w / 2;
        dst.y = static_cast<int>(y) * rs.wh / rs.ah - dst.h / 2;
        SDL_SetTextureAlphaMod(starTexture, static_cast<Uint8>(std::clamp(alpha, 0, 255)));
        SDL_RenderCopy(renderer, starTexture, nullptr, &dst);
    }
}

void EvolutionScene::drawOverlay(RenderSizes rs) const
{
    if (flashAlpha <= 0 && fadeAlpha <= 0)
        return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (flashAlpha > 0)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(std::clamp(flashAlpha, 0, 255)));
        SDL_RenderFillRect(renderer, nullptr);
    }
    if (fadeAlpha > 0)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(std::clamp(fadeAlpha, 0, 255)));
        SDL_RenderFillRect(renderer, nullptr);
    }
    (void)rs;
}

void EvolutionScene::draw(Fps const* fps, RenderSizes rs)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawBackground(rs);
    drawMon(rs);
    drawSparkles(rs);
    drawOverlay(rs);

    if (phase == Phase::IntroMsg || phase == Phase::CongratsMsg || phase == Phase::CancelMsg)
        speech->draw(fps, rs);
}

bool EvolutionScene::popScene() const
{
    return leave;
}

std::string EvolutionScene::name()
{
    return "EvolutionScene";
}
