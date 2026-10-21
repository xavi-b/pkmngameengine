#ifndef SINGLEBATTLESCENE_H
#define SINGLEBATTLESCENE_H

#include "battles/battleactions.h"
#include "battles/battlebackground.h"
#include "battles/battlespeech.h"
#include "battles/moveselection.h"
#include "battles/singlebattleui.h"
#include "phases/singlebattlephases.h"
#include "scene.h"
#include "singlebattlescenedata.h"

#include <SDL_image.h>

class WeatherAnimation;

struct DamageAdjustments
{
    bool  useTargetAttack     = false;
    bool  useUserDefense      = false;
    bool  usePhysicalDefense  = false;
    bool  forcePhysical       = false;
    bool  forceSpecial        = false;
    bool  ignoreDefenseStages = false;
    bool  ignoreAttackStages  = false;
    bool  includeFlying       = false;
    bool  pierceGroundImmunity = false;
    bool  blockCrit           = false;
    float multiplier          = 1.f;
    float stabBonus           = 0.f;
};

class SingleBattleScene : public Scene
{
public:
    using State = BattleState;

    friend class SingleBattlePhase;

    SingleBattleScene(SDL_Renderer* renderer);
    virtual ~SingleBattleScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;
    virtual bool pushScene() const override;
    virtual void popReset() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

    void changeWeather(Map::Weather weather);
    void applyFieldWeather(Map::Weather weather);
    WeatherAnimation* fieldWeatherAnim() const;

    void setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn, bool deferSwitchInAbility = false);
    void setOpponentPkmn(Pkmn::PkmnPtr const& newOpponentPkmn, bool deferSwitchInAbility = false);
    // True when an Eject Pack switch changed the battle state.
    bool consumeEjectSwitch();
    void resolveOpponentFaint();

    struct DamageRoll
    {
        size_t amount         = 0;
        bool   critical       = false;
        float  effectiveness  = 1.f;
    };

    static DamageRoll computeDamage(Pkmn::PkmnPtr const& attacker,
                                    Pkmn::PkmnPtr const& defender,
                                    Move::MovePtr const& move,
                                    Map::Weather const&  weather,
                                    BattleBattler const* attackerBattler = nullptr,
                                    BattleBattler const* defenderBattler = nullptr,
                                    size_t               powerOverride   = 0,
                                    bool                 forceNoCrit       = false,
                                    bool                 forceCrit         = false,
                                    float                screenMultiplier = 1.f,
                                    bool                 wonderRoom       = false,
                                    std::string const&   typeOverride     = {},
                                    DamageAdjustments    adjustments  = {});

protected:
    virtual std::string encounterStartText() const    = 0;
    virtual bool                                 opponentSentFromBall() const;
    virtual std::string                          opponentTrainerGraphic() const;
    virtual std::array<SingleBattleUi::LineupBall, 6> opponentLineup() const;

    std::shared_ptr<SingleBattleSceneData> data;

    std::unique_ptr<BattleBackground> battleBackground;
    std::unique_ptr<SingleBattleUi>   singleBattleUi;
    std::unique_ptr<WeatherAnimation> fieldWeather;

    SingleBattlePhase* currentPhase();
    virtual void       initPhases();

    std::unique_ptr<SingleBattleStartPhase>         startPhase;
    std::unique_ptr<SingleBattleWeatherPhase>       weatherPhase;
    std::unique_ptr<SingleBattleActionsPhase>       actionsPhase;
    std::unique_ptr<SingleBattleMovesPhase>         movesPhase;
    std::unique_ptr<SingleBattlePlayerMovesPhase>   playerMovesPhase;
    std::unique_ptr<SingleBattlePlayerItemsPhase>   playerItemsPhase;
    std::unique_ptr<SingleBattlePlayerPkmnsPhase>   playerPkmnsPhase;
    std::unique_ptr<SingleBattlePlayerRunPhase>     playerRunPhase;
    std::unique_ptr<SingleBattleOpponentPkmnsPhase> opponentPkmnsPhase;
    std::unique_ptr<SingleBattleOpponentItemsPhase> opponentItemsPhase;
    std::unique_ptr<SingleBattleOpponentMovesPhase> opponentMovesPhase;
    std::unique_ptr<SingleBattleOpponentRunPhase>   opponentRunPhase;
    std::unique_ptr<SingleBattleExperiencePhase>    experiencePhase;
    std::unique_ptr<SingleBattleEndPhase>           endPhase;
};

#endif // SINGLEBATTLESCENE_H
