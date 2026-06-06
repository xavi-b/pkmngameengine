#include "singlebattleendphase.h"

#include "../singlebattlescene.h"
#include "animations/fadeanimation.h"

SingleBattleEndPhase::SingleBattleEndPhase(SingleBattleScene&                     scene,
                                           std::shared_ptr<SingleBattleSceneData> data,
                                           SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

bool SingleBattleEndPhase::isFinished() const
{
    return fadeOutAnimation && fadeOutAnimation->isFinished();
}

void SingleBattleEndPhase::update(Inputs const* /*inputs*/)
{
    if (!fadeOutAnimation)
        fadeOutAnimation = std::make_unique<FadeAnimation>(renderer, false);

    if (!fadeOutAnimation->isStarted())
    {
        fadeOutAnimation->start();
    }

    if (!fadeOutAnimation->isFinished())
    {
        fadeOutAnimation->incrementTicks();
    }
}

void SingleBattleEndPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (fadeOutAnimation)
        fadeOutAnimation->draw(fps, rs);
}
