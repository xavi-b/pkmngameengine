#include "singlebattlephase.h"

#include "../singlebattlescene.h"
#include "battles/singlebattleui.h"
#include "utils.h"

SingleBattlePhase::SingleBattlePhase(SingleBattleScene&                     scene,
                                     std::shared_ptr<SingleBattleSceneData> data,
                                     SDL_Renderer*                          renderer)
    : scene(scene), data(data), renderer(renderer)
{
}

void SingleBattlePhase::preupdate()
{
}

SingleBattleUi* SingleBattlePhase::battleUi()
{
    return scene.singleBattleUi.get();
}

bool SingleBattlePhase::foeSentFromBall() const
{
    return scene.opponentSentFromBall();
}

std::string SingleBattlePhase::opponentTrainerGraphic() const
{
    return scene.opponentTrainerGraphic();
}

std::array<SingleBattleUi::LineupBall, 6> SingleBattlePhase::opponentLineup() const
{
    return scene.opponentLineup();
}

bool SingleBattlePhase::playFaint(bool playerSide)
{
    auto* ui = battleUi();
    if (ui->hasFainted(playerSide))
        return true;
    if (!ui->isFaintRunning())
        ui->startFaint(playerSide);
    else
        ui->incrementFaint();
    return ui->hasFainted(playerSide);
}