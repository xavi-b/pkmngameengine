#ifndef EVOLUTIONSCENE_H
#define EVOLUTIONSCENE_H

#include "pkmn.h"
#include "scene.h"
#include "textspeech.h"

#include <string>
#include <vector>

class EvolutionScene : public Scene
{
public:
    EvolutionScene(SDL_Renderer* renderer, Pkmn::PkmnPtr const& pkmn, PkmnDef::PkmnDefPtr const& evolutionDef);
    virtual ~EvolutionScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::string name() override;

private:
    enum class Phase
    {
        FadeIn,
        IntroMsg,
        IntroCry,
        IntroJingle,
        StartMusic,
        SparkleSpiral,
        SparkleArc,
        CycleSprites,
        SparkleCircle,
        SparkleSpray,
        EvoSound,
        RestoreScreen,
        EvoCry,
        CongratsMsg,
        CancelFlash,
        CancelCry,
        CancelMsg,
        FadeOut
    };

    enum class SparkleKind
    {
        SpiralUp,
        ArcDown,
        CircleIn,
        Spray
    };

    struct Sparkle
    {
        SparkleKind kind   = SparkleKind::SpiralUp;
        int         angle  = 0;
        int         radius = 0;
        int         t      = 0;
        int         shrink = 4;
        int         drift  = 0;
        bool        alive  = true;
    };

    struct SpeciesSprite
    {
        SDL_Texture* color = nullptr;
        SDL_Texture* white = nullptr;
        int          w     = 0;
        int          h     = 0;
    };

    void enter(Phase next);
    bool stepFrame(Inputs const* inputs);
    void stepSparkles();
    void spawnSpiral(int angle);
    void spawnArc(int angle);
    void spawnCircle(int angle, int shrink);
    void spawnSpray();

    void showMessage(std::string const& text);
    void commitEvolution();

    SpeciesSprite loadSpeciesSprite(std::string const& speciesId) const;
    void          destroySprite(SpeciesSprite& sprite);

    void drawBackground(RenderSizes rs);
    void drawMon(RenderSizes rs);
    void drawSparkles(RenderSizes rs);
    void drawOverlay(RenderSizes rs) const;

    bool sparklePosition(Sparkle const& sparkle, float& x, float& y, float& scale, int& alpha) const;

    Pkmn::PkmnPtr       pkmn;
    PkmnDef::PkmnDefPtr evolutionDef;
    std::string         oldName;
    Phase               phase      = Phase::FadeIn;
    int                 phaseFrame = 0;
    bool                evolved    = false;
    bool                leave      = false;

    SpeciesSprite preSprite;
    SpeciesSprite postSprite;

    SDL_Surface* bgSurface   = nullptr;
    SDL_Texture* bgTexture   = nullptr;
    SDL_Texture* starTexture = nullptr;

    Mix_Music* evolutionMusic = nullptr;
    Mix_Music* successMusic   = nullptr;
    Mix_Music* startJingle    = nullptr;
    Mix_Chunk* spiralSe       = nullptr;
    Mix_Chunk* arcSe          = nullptr;
    Mix_Chunk* circleSe       = nullptr;
    Mix_Chunk* spraySe        = nullptr;
    Mix_Chunk* poofSe         = nullptr;

    std::unique_ptr<TextSpeech> speech;
    std::vector<Sparkle>        sparkles;

    int    preScale      = 256;
    int    postScale     = 16;
    bool   cycleGrowPost = false;
    bool   cyclePulsing  = false;
    int    cycleSpeed    = 8;
    int    bgAlpha       = 0;
    int    bgAngle       = 0;
    int    spriteWhite   = 0;
    int    flashAlpha    = 0;
    int    fadeAlpha     = 255;
    bool   showPost      = false;
    Uint32 clockMs       = 0;
    int    msAccum       = 0;
};

#endif // EVOLUTIONSCENE_H
