#ifndef SINGLEBATTLEOPPONENTMOVESPHASE_H
#define SINGLEBATTLEOPPONENTMOVESPHASE_H

#include "pkmn.h"
#include "singlebattlephase.h"
#include "textspeech.h"

#include <string>
#include <vector>

class SingleBattleOpponentMovesPhase : public SingleBattlePhase
{
public:
    SingleBattleOpponentMovesPhase(SingleBattleScene&                     scene,
                                   std::shared_ptr<SingleBattleSceneData> data,
                                   SDL_Renderer*                          renderer);

    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string opponentMoveText(Move::MovePtr const& move) const = 0;

private:
    enum class Step
    {
        MajorStatus,
        Flinch,
        Confusion,
        Attraction,
        AnnounceMove,
        ExecuteMove,
        Advance,
        GoToPkmns,
        GoToOpponentPkmns,
        GoToOpponentRun,
        GoToEnd,
        GoToExperience,
        GoToPlayerFaint,
        AwaitFaint,
        AwaitOpponentConfusionFaint
    };

    void showSpeech(std::vector<std::string> const& texts);
    void advanceAfterAction();
    void executeOpponentMove();
    void continueAfterOpponentMove();
    void handlePlayerFaint();

    Step                        step = Step::MajorStatus;
    std::unique_ptr<TextSpeech> speech;
    std::vector<std::string>    pendingMessages;
    bool                        pendingPlayerFaint   = false;
    bool                        pendingOpponentFaint = false;
    bool                        pendingSwitchUser    = false;
    bool                        pendingSwitchTarget  = false;
};

#endif // SINGLEBATTLEOPPONENTMOVESPHASE_H
