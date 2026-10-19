#ifndef SINGLEBATTLEPHASE_H
#define SINGLEBATTLEPHASE_H

#include "../singlebattlescenedata.h"
#include "battles/singlebattleui.h"
#include "fps.h"
#include "inputs.h"
#include "renderutils.h"

#include <array>

#include <memory>

class SingleBattleScene;

struct SingleBattlePhaseContext
{
    SDL_Renderer* renderer;

    class BattleBackground* battleBackground;
    class BattleSpeech*     battleSpeech;
    class BattleActions*    battleActions;
    class SingleBattleUi*   singleBattleUi;
    class MoveSelection*    moveSelection;
};

class SingleBattlePhase
{
public:
    SingleBattlePhase(SingleBattleScene& scene, std::shared_ptr<SingleBattleSceneData> data, SDL_Renderer* renderer);
    virtual ~SingleBattlePhase() = default;

    virtual void preupdate();
    virtual void update(Inputs const* inputs)         = 0;
    virtual void draw(Fps const* fps, RenderSizes rs) = 0;

protected:
    class SingleBattleUi*                     battleUi();
    bool                                      foeSentFromBall() const;
    std::string                               opponentTrainerGraphic() const;
    std::array<SingleBattleUi::LineupBall, 6> opponentLineup() const;

    // Plays the faint drop for one side. Returns true once that sprite has sunk away.
    bool playFaint(bool playerSide);

    SingleBattleScene&                     scene;
    std::shared_ptr<SingleBattleSceneData> data;
    SDL_Renderer*                          renderer;
};

#endif // SINGLEBATTLEPHASE_H
