#ifndef ENCOUNTERSCENE_H
#define ENCOUNTERSCENE_H

#include "battlescene.h"
#include "item.h"
#include "pkmn.h"

#include <SDL_image.h>
#include <set>

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

    virtual void chooseOpponentAction();

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

    virtual void update_EXPERIENCE(Inputs const* inputs) override;
    virtual void draw_EXPERIENCE(Fps const* fps, RenderSizes rs) override;

    virtual void update_END(Inputs const* inputs) override;
    virtual void draw_END(Fps const* fps, RenderSizes rs) override;

protected:
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
    std::unique_ptr<TextSpeech> noPpLeftSpeech;
    std::unique_ptr<TextSpeech> experienceSpeech;
    std::unique_ptr<TextSpeech> moveToLearnSpeech;
    std::unique_ptr<TextSpeech> playerMoveSpeech;
    std::unique_ptr<TextSpeech> opponentMoveSpeech;
    std::unique_ptr<TextSpeech> endSpeech;
    std::unique_ptr<TextSpeech> pkmnEnterSpeech;
    std::unique_ptr<TextSpeech> pkmnFaintSpeech;

    BattleActions::Type             opponentAction  = BattleActions::Type::MOVES;
    Pkmn::PkmnPtr                   newSelectedPkmn = nullptr;
    Item::ItemPtr                   selectedItem    = nullptr;
    std::set<Pkmn::PkmnPtr>         participatingPlayerPkmns;
    std::map<Pkmn::PkmnPtr, size_t> expGained;

    // Current pkmn experience gain information
    bool                shouldBreakToEvolution  = false;
    bool                shouldBreakToLevelUp    = false;
    bool                shouldBreakToNewMove    = false;
    bool                shouldGoToNewMovesScene = false;
    Pkmn::PkmnPtr       expPkmn                 = nullptr;
    size_t              expFromBattle           = 0;
    PkmnDef::PkmnDefPtr evolutionDef            = nullptr;
};

#endif // ENCOUNTERSCENE_H
