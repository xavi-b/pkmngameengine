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