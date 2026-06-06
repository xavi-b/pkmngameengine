#include "singlebattleplayerpkmnsphase.h"

#include "../singlebattlescene.h"
#include "battles/battleactions.h"
#include "textspeech.h"

SingleBattlePlayerPkmnsPhase::SingleBattlePlayerPkmnsPhase(SingleBattleScene&                     scene,
                                                           std::shared_ptr<SingleBattleSceneData> data,
                                                           SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattlePlayerPkmnsPhase::showEnterSpeech(std::string const& text)
{
    pkmnEnterSpeech = std::make_unique<TextSpeech>(renderer);
    pkmnEnterSpeech->setTexts({text});
    pkmnEnterSpeech->start();
}

void SingleBattlePlayerPkmnsPhase::update(Inputs const* inputs)
{
    if (pkmnEnterSpeech)
    {
        pkmnEnterSpeech->update(inputs);
        if (pkmnEnterSpeech->shouldClose())
            pkmnEnterSpeech.release();
        return;
    }

    scene.setPlayerPkmn(data->newSelectedPkmn);
    data->newSelectedPkmn.reset();

    // TODO: Pkmn animation

    data->chooseOpponentAction();

    data->playerFirst = true;

    switch (data->opponentAction)
    {
    case BattleActions::BAG:
        data->state = OPPONENT_ITEMS;
        break;
    case BattleActions::PKMNS:
        data->state = OPPONENT_PKMNS;
        break;
    case BattleActions::RUN:
        data->state = OPPONENT_RUN;
        break;
    case BattleActions::MOVES:
    default:
        data->state = OPPONENT_MOVES;
        break;
    }
}

void SingleBattlePlayerPkmnsPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (pkmnEnterSpeech)
    {
        pkmnEnterSpeech->draw(fps, rs);
        return;
    }
}
