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

    virtual bool pushScene() const override;
    virtual void popReset() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

    void setEncounterPkmn(Pkmn::PkmnPtr const& newEncounterPkmn);
    void setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn);

    void chooseOpponentAction();

    virtual void update_START(Inputs const* inputs) override;
    virtual void draw_START(Fps const* fps, RenderSizes rs) override;

    virtual void update_WEATHER(Inputs const* inputs) override;
    virtual void draw_WEATHER(Fps const* fps, RenderSizes rs) override;

    virtual void update_ACTIONS(Inputs const* inputs) override;
    virtual void draw_ACTIONS(Fps const* fps, RenderSizes rs) override;

    virtual void update_MOVES(Inputs const* inputs) override;
    virtual void draw_MOVES(Fps const* fps, RenderSizes rs) override;

    virtual void update_PLAYER_MOVES(Inputs const* inputs) override;
    virtual void draw_PLAYER_MOVES(Fps const* fps, RenderSizes rs) override;

    virtual void update_PLAYER_ITEMS(Inputs const* inputs) override;
    virtual void draw_PLAYER_ITEMS(Fps const* fps, RenderSizes rs) override;

    virtual void update_PLAYER_PKMNS(Inputs const* inputs) override;
    virtual void draw_PLAYER_PKMNS(Fps const* fps, RenderSizes rs) override;

    virtual void update_PLAYER_RUN(Inputs const* inputs) override;
    virtual void draw_PLAYER_RUN(Fps const* fps, RenderSizes rs) override;

    virtual void update_OPPONENT_PKMNS(Inputs const* inputs) override;
    virtual void draw_OPPONENT_PKMNS(Fps const* fps, RenderSizes rs) override;

    virtual void update_OPPONENT_ITEMS(Inputs const* inputs) override;
    virtual void draw_OPPONENT_ITEMS(Fps const* fps, RenderSizes rs) override;

    virtual void update_OPPONENT_MOVES(Inputs const* inputs) override;
    virtual void draw_OPPONENT_MOVES(Fps const* fps, RenderSizes rs) override;

    virtual void update_OPPONENT_RUN(Inputs const* inputs) override;
    virtual void draw_OPPONENT_RUN(Fps const* fps, RenderSizes rs) override;

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
    std::unique_ptr<TextSpeech> runSpeech;
    std::unique_ptr<TextSpeech> failedRunSpeech;

    BattleActions::Type opponentAction  = BattleActions::Type::MOVES;
    Pkmn::PkmnPtr       newSelectedPkmn = nullptr;
};

#endif // ENCOUNTERSCENE_H
