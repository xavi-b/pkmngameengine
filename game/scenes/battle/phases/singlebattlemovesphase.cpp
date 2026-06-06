#include "singlebattlemovesphase.h"

#include "../singlebattlescene.h"
#include "battles/battleactions.h"
#include "game.h"
#include "textspeech.h"
#include "utils.h"

#include <boost/format.hpp>

SingleBattleMovesPhase::SingleBattleMovesPhase(SingleBattleScene&                     scene,
                                               std::shared_ptr<SingleBattleSceneData> data,
                                               SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleMovesPhase::init()
{
    moveSelection = std::make_unique<MoveSelection>(renderer);
    moveSelection->init();
}

void SingleBattleMovesPhase::preupdate()
{
    moveSelection->reset();
    moveSelection->setPkmn(data->playerPkmn);
}

void SingleBattleMovesPhase::update(Inputs const* inputs)
{
    if (noPpLeftSpeech)
    {
        noPpLeftSpeech->update(inputs);
        if (noPpLeftSpeech->shouldClose())
            noPpLeftSpeech.release();
        return;
    }

    if (moveSelection->shouldQuit())
    {
        data->state = ACTIONS;
    }
    else if (moveSelection->isSelected())
    {
        data->playerMove = data->playerPkmn->getMoves().at(moveSelection->selectedIndex());
        if (data->playerMove && data->playerMove->getCurrentPP() > 0)
        {
            data->chooseOpponentAction();

            if (data->opponentAction == BattleActions::Type::MOVES)
            {
                if (!data->encounterMove)
                    data->playerFirst = true;
                else if (data->playerMove->getDefinition()->getPriority()
                         > data->encounterMove->getDefinition()->getPriority())
                    data->playerFirst = true;
                else if (data->playerMove->getDefinition()->getPriority()
                         < data->encounterMove->getDefinition()->getPriority())
                    data->playerFirst = false;
                else
                {
                    size_t playerSpeed    = data->playerPkmn->getStats()[PkmnDef::SPEED];
                    size_t encounterSpeed = data->opponentPkmn->getStats()[PkmnDef::SPEED];
                    if (playerSpeed > encounterSpeed)
                        data->playerFirst = true;
                    else if (playerSpeed < encounterSpeed)
                        data->playerFirst = false;
                    else
                        data->playerFirst = Utils::randuint(0, 1);
                }
            }
            else
            {
                data->playerFirst = false;
            }

            if (data->playerFirst)
            {
                data->state = PLAYER_MOVES;
            }
            else
            {
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
        }
        else
        {
            noPpLeftSpeech = std::make_unique<TextSpeech>(renderer);
            noPpLeftSpeech->setTexts({lc::translate("There's no PP left for this move !")});
            noPpLeftSpeech->start();
            moveSelection->reset();
        }
    }
    else
    {
        moveSelection->update(inputs);
    }
}

void SingleBattleMovesPhase::draw(Fps const* fps, RenderSizes rs)
{
    moveSelection->draw(fps, rs);
    if (noPpLeftSpeech)
        noPpLeftSpeech->draw(fps, rs);
}
