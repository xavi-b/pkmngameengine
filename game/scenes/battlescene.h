#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "battles/battleactions.h"
#include "battles/battlebackground.h"
#include "battles/battlespeech.h"
#include "battles/moveselection.h"
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
    virtual bool pushScene() const override;
    virtual void popReset() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

    virtual size_t computeDamage(Pkmn::PkmnPtr const& pkmn, Move::MovePtr const& move) const;
    std::string    canEvolve(Pkmn::PkmnPtr const& pkmn);

    virtual void update_START(Inputs const* inputs)        = 0;
    virtual void draw_START(Fps const* ps, RenderSizes rs) = 0;

    virtual void update_ACTIONS(Inputs const* inputs)        = 0;
    virtual void draw_ACTIONS(Fps const* ps, RenderSizes rs) = 0;

    virtual void update_P_MOVES(Inputs const* inputs)        = 0;
    virtual void draw_P_MOVES(Fps const* ps, RenderSizes rs) = 0;

    virtual void update_P_ITEMS(Inputs const* inputs)        = 0;
    virtual void draw_P_ITEMS(Fps const* ps, RenderSizes rs) = 0;

    virtual void update_P_PKMNS(Inputs const* inputs)        = 0;
    virtual void draw_P_PKMNS(Fps const* ps, RenderSizes rs) = 0;

    virtual void update_P_RUN(Inputs const* inputs)        = 0;
    virtual void draw_P_RUN(Fps const* ps, RenderSizes rs) = 0;

    virtual void update_O_PKMNS(Inputs const* inputs)        = 0;
    virtual void draw_O_PKMNS(Fps const* ps, RenderSizes rs) = 0;

    virtual void update_O_ITEMS(Inputs const* inputs)        = 0;
    virtual void draw_O_ITEMS(Fps const* ps, RenderSizes rs) = 0;

    virtual void update_O_MOVES(Inputs const* inputs)        = 0;
    virtual void draw_O_MOVES(Fps const* ps, RenderSizes rs) = 0;

protected:
    enum State
    {
        START,
        ACTIONS,
        BAG,
        PKMNS,
        PLAYER_MOVES,
        PLAYER_ITEMS,
        PLAYER_PKMNS,
        PLAYER_RUN,
        OPPONENT_PKMNS,
        OPPONENT_ITEMS,
        OPPONENT_MOVES,
        END
    };

    static std::string StateToString(State e);

    std::unique_ptr<BattleBackground> battleBackground;
    std::unique_ptr<BattleSpeech>     battleSpeech;
    std::unique_ptr<BattleActions>    battleActions;
    std::unique_ptr<MoveSelection>    moveSelection;

    State state         = START;
    State previousState = END;
};

#endif // BATTLESCENE_H
