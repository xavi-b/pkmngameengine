#include "singlebattleplayerrunphase.h"

#include "../singlebattlescene.h"
#include "game.h"
#include "textspeech.h"
#include "utils.h"

#include <iostream>

SingleBattlePlayerRunPhase::SingleBattlePlayerRunPhase(SingleBattleScene&                     scene,
                                                       std::shared_ptr<SingleBattleSceneData> data,
                                                       SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattlePlayerRunPhase::update(Inputs const* inputs)
{
    if (!canPlayerRun())
    {
        if (!failedRunSpeech)
        {
            failedRunSpeech = std::make_unique<TextSpeech>(renderer);
            failedRunSpeech->setTexts({lc::translate("No! There's no running from a trainer battle !")});
            failedRunSpeech->start();
        }

        failedRunSpeech->update(inputs);
        if (failedRunSpeech->shouldClose())
        {
            failedRunSpeech.release();
            data->state = ACTIONS;
        }
        return;
    }

    if (runSpeech)
    {
        runSpeech->update(inputs);
        if (runSpeech->shouldClose())
        {
            runSpeech.release();
            data->state = END;
        }
        return;
    }

    if (failedRunSpeech)
    {
        failedRunSpeech->update(inputs);
        if (failedRunSpeech->shouldClose())
        {
            failedRunSpeech.release();

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
        return;
    }

    bool run = tryPlayerRun();

    if (Game::instance()->isDebug())
        std::cout << __PRETTY_FUNCTION__ << " run: " << run << std::endl;

    if (run)
    {
        runSpeech = std::make_unique<TextSpeech>(renderer);
        runSpeech->setTexts({lc::translate("Got away safely !")});
        runSpeech->start();
    }
    else
    {
        failedRunSpeech = std::make_unique<TextSpeech>(renderer);
        failedRunSpeech->setTexts({lc::translate("Can't get away !")});
        failedRunSpeech->start();
    }
}

void SingleBattlePlayerRunPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (runSpeech)
    {
        runSpeech->draw(fps, rs);
        return;
    }

    if (failedRunSpeech)
    {
        failedRunSpeech->draw(fps, rs);
        return;
    }
}
