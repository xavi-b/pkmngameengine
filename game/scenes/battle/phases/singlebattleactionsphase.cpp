#include "singlebattleactionsphase.h"

#include "../singlebattlescene.h"
#include "battles/battleactions.h"
#include "game.h"
#include "utils.h"

SingleBattleActionsPhase::SingleBattleActionsPhase(SingleBattleScene&                     scene,
                                                   std::shared_ptr<SingleBattleSceneData> data,
                                                   SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleActionsPhase::init()
{
    battleSpeech = std::make_unique<BattleSpeech>(renderer);
    boost::format whatShouldDoText =
        boost::format(lc::translate("What should %1% do ?")) % Game::instance()->data.player.name;
    battleSpeech->setTexts({whatShouldDoText.str()});
    battleSpeech->init();

    battleActions = std::make_unique<BattleActions>(renderer);
    battleActions->init();
}

void SingleBattleActionsPhase::preupdate()
{
    battleActions->reset();
}

void SingleBattleActionsPhase::update(Inputs const* inputs)
{
    battleSpeech->update(inputs);
    if (battleSpeech->mayClose())
    {
        if (battleActions->isFinished())
        {
            switch (battleActions->selectedAction())
            {
            case BattleActions::MOVES: {
                data->state = MOVES;
                break;
            }
            case BattleActions::BAG:
                data->state = BAG;
                break;
            case BattleActions::PKMNS:
                data->state = PKMNS;
                break;
            case BattleActions::RUN:
                data->state = PLAYER_RUN;
                break;
            default:
                break;
            }
        }
        else
        {
            battleActions->update(inputs);
        }
    }
}

void SingleBattleActionsPhase::draw(Fps const* fps, RenderSizes rs)
{
    battleSpeech->draw(fps, rs);
    battleActions->draw(fps, rs);
}
