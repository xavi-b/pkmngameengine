#include "singlebattleopponentrunphase.h"

#include "textspeech.h"

#include <string>

SingleBattleOpponentRunPhase::SingleBattleOpponentRunPhase(SingleBattleScene&                     scene,
                                                           std::shared_ptr<SingleBattleSceneData> data,
                                                           SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleOpponentRunPhase::update(Inputs const* inputs)
{
    if (!runSpeech)
    {
        runSpeech = std::make_unique<TextSpeech>(renderer);
        runSpeech->setTexts({opponentRunText()});
        runSpeech->start();
    }

    runSpeech->update(inputs);
    if (runSpeech->shouldClose())
    {
        runSpeech.release();
        data->state = END;
    }
}

void SingleBattleOpponentRunPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (runSpeech)
    {
        runSpeech->draw(fps, rs);
    }
}
