#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "animations/eyeanimation.h"
#include "animations/fadeanimation.h"
#include "battles/battleactions.h"
#include "battles/battlebackground.h"
#include "battles/battlespeech.h"
#include "battles/moveselection.h"
#include "battles/singlebattleui.h"
#include "pkmn.h"
#include "scene.h"

#include <SDL_image.h>

class BattleScene : public Scene
{
public:
    BattleScene(SDL_Renderer* renderer);
    virtual ~BattleScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::string name() override;

    virtual size_t computeDamage(Pkmn::PkmnPtr const& pkmn, Move::MovePtr const& move) const;
    std::string    canEvolve(Pkmn::PkmnPtr const& pkmn);

    virtual void update_START(Inputs const* inputs)         = 0;
    virtual void draw_START(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_ACTIONS(Inputs const* inputs)         = 0;
    virtual void draw_ACTIONS(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_MOVES(Inputs const* inputs)         = 0;
    virtual void draw_MOVES(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_P_MOVES(Inputs const* inputs)         = 0;
    virtual void draw_P_MOVES(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_P_ITEMS(Inputs const* inputs)         = 0;
    virtual void draw_P_ITEMS(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_P_PKMNS(Inputs const* inputs)         = 0;
    virtual void draw_P_PKMNS(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_P_RUN(Inputs const* inputs)         = 0;
    virtual void draw_P_RUN(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_O_PKMNS(Inputs const* inputs)         = 0;
    virtual void draw_O_PKMNS(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_O_ITEMS(Inputs const* inputs)         = 0;
    virtual void draw_O_ITEMS(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_O_MOVES(Inputs const* inputs)         = 0;
    virtual void draw_O_MOVES(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_O_RUN(Inputs const* inputs)         = 0;
    virtual void draw_O_RUN(Fps const* fps, RenderSizes rs) = 0;

    virtual void update_END(Inputs const* inputs)         = 0;
    virtual void draw_END(Fps const* fps, RenderSizes rs) = 0;

protected:
    enum State
    {
        START,
        ACTIONS,
        MOVES,
        BAG,
        PKMNS,
        PLAYER_MOVES,
        PLAYER_ITEMS,
        PLAYER_PKMNS,
        PLAYER_RUN,
        OPPONENT_PKMNS,
        OPPONENT_ITEMS,
        OPPONENT_MOVES,
        OPPONENT_RUN,
        END
    };

    static std::string StateToString(State e);

    std::unique_ptr<BattleBackground> battleBackground;
    std::unique_ptr<BattleSpeech>     battleSpeech;
    std::unique_ptr<BattleActions>    battleActions;
    std::unique_ptr<SingleBattleUi>   singleBattleUi;
    std::unique_ptr<MoveSelection>    moveSelection;
    std::unique_ptr<FadeAnimation>    fadeOutAnimation;
    std::unique_ptr<EyeAnimation>     eyeAnimation;

    State state         = START;
    State previousState = END;
};

#endif // BATTLESCENE_H
