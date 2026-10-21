#ifndef SINGLEBATTLEPLAYERMOVESPHASE_H
#define SINGLEBATTLEPLAYERMOVESPHASE_H

#include "singlebattlephase.h"
#include "textspeech.h"

#include <vector>

class SingleBattlePlayerMovesPhase : public SingleBattlePhase
{
public:
    SingleBattlePlayerMovesPhase(SingleBattleScene&                     scene,
                                 std::shared_ptr<SingleBattleSceneData> data,
                                 SDL_Renderer*                          renderer);

    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

    virtual float battleExperienceMultiplier() const = 0;
    virtual void  onOpponentPkmnDefeated()           = 0;

    void resolveOpponentFaint();

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
        AwaitFaint,
        AwaitPlayerConfusionFaint
    };

    void showSpeech(std::vector<std::string> const& texts);
    void awardExperienceForOpponentKo();
    void advanceAfterAction();
    void executePlayerMove();
    void continueAfterPlayerMove();
    void handlePlayerFaintFromMove(std::vector<std::string> texts);

    Step                        step = Step::MajorStatus;
    std::unique_ptr<TextSpeech> speech;
    std::vector<std::string>    pendingMessages;
    bool                        pendingPlayerFaint   = false;
    bool                        pendingOpponentFaint = false;
    bool                        pendingSwitchUser    = false;
    bool                        pendingSwitchTarget  = false;
};

#endif // SINGLEBATTLEPLAYERMOVESPHASE_H
