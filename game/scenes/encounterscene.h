#ifndef ENCOUNTERSCENE_H
#define ENCOUNTERSCENE_H

#include "battlescene.h"
#include "pkmn.h"

#include <SDL_image.h>

class EncounterScene : public BattleScene
{
public:
    EncounterScene(SDL_Renderer* renderer);
    virtual ~EncounterScene();

    virtual void init() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string name() override;

    void setEncounterPkmn(Pkmn::PkmnPtr const& newEncounterPkmn);
    void setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn);

    void chooseOpponentAction();

    virtual void update_START(Inputs const* inputs) override;
    virtual void draw_START(Fps const* fps, RenderSizes rs) override;

    virtual void update_ACTIONS(Inputs const* inputs) override;
    virtual void draw_ACTIONS(Fps const* fps, RenderSizes rs) override;

    virtual void update_MOVES(Inputs const* inputs) override;
    virtual void draw_MOVES(Fps const* fps, RenderSizes rs) override;

    virtual void update_P_MOVES(Inputs const* inputs) override;
    virtual void draw_P_MOVES(Fps const* fps, RenderSizes rs) override;

    virtual void update_P_ITEMS(Inputs const* inputs) override;
    virtual void draw_P_ITEMS(Fps const* fps, RenderSizes rs) override;

    virtual void update_P_PKMNS(Inputs const* inputs) override;
    virtual void draw_P_PKMNS(Fps const* fps, RenderSizes rs) override;

    virtual void update_P_RUN(Inputs const* inputs) override;
    virtual void draw_P_RUN(Fps const* fps, RenderSizes rs) override;

    virtual void update_O_PKMNS(Inputs const* inputs) override;
    virtual void draw_O_PKMNS(Fps const* fps, RenderSizes rs) override;

    virtual void update_O_ITEMS(Inputs const* inputs) override;
    virtual void draw_O_ITEMS(Fps const* fps, RenderSizes rs) override;

    virtual void update_O_MOVES(Inputs const* inputs) override;
    virtual void draw_O_MOVES(Fps const* fps, RenderSizes rs) override;

    virtual void update_O_RUN(Inputs const* inputs) override;
    virtual void draw_O_RUN(Fps const* fps, RenderSizes rs) override;

    virtual void update_END(Inputs const* inputs) override;
    virtual void draw_END(Fps const* fps, RenderSizes rs) override;

private:
    Pkmn::PkmnPtr encounterPkmn;
    Pkmn::PkmnPtr playerPkmn;
    size_t        runAttemps    = 0;
    Move::MovePtr encounterMove = nullptr;
    Move::MovePtr playerMove    = nullptr;
    bool          playerFirst   = true;

    std::unique_ptr<TextSpeech> pkmnEncounterSpeech;
    std::unique_ptr<TextSpeech> firstPkmnSpeech;

    BattleActions::Type opponentAction = BattleActions::Type::MOVES;
};

#endif // ENCOUNTERSCENE_H
