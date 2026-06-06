#include "singlebattleopponentmovesphase.h"

#include "../singlebattlescene.h"
#include "game.h"
#include "textspeech.h"
#include "utils.h"

#include <boost/format.hpp>
#include <iostream>

SingleBattleOpponentMovesPhase::SingleBattleOpponentMovesPhase(SingleBattleScene&                     scene,
                                                               std::shared_ptr<SingleBattleSceneData> data,
                                                               SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleOpponentMovesPhase::preupdate()
{
    if (data->encounterMove)
    {
        auto text          = opponentMoveText(data->encounterMove);
        opponentMoveSpeech = std::make_unique<TextSpeech>(renderer);
        opponentMoveSpeech->setTexts({text});
        opponentMoveSpeech->start();
    }
    else
    {
        opponentMoveSpeech.release();
    }
}

void SingleBattleOpponentMovesPhase::update(Inputs const* inputs)
{
    if (!data->encounterMove || data->encounterMove->getCurrentPP() == 0)
    {
        data->state = data->playerFirst ? WEATHER : PLAYER_MOVES;
        return;
    }

    if (Game::instance()->isDebug())
        std::cout << __PRETTY_FUNCTION__ << " Move: " << data->encounterMove->getDefinition()->getId() << std::endl;

    if (opponentMoveSpeech)
    {
        opponentMoveSpeech->update(inputs);
        if (opponentMoveSpeech->shouldClose())
            opponentMoveSpeech.release();
        return;
    }

    if (pkmnFaintSpeech)
    {
        pkmnFaintSpeech->update(inputs);
        if (pkmnFaintSpeech->shouldClose())
        {
            pkmnFaintSpeech.release();
            data->state = PKMNS;
        }
        return;
    }

    if (endSpeech)
    {
        endSpeech->update(inputs);
        if (endSpeech->shouldClose())
        {
            endSpeech.release();
            data->state = END;
        }
        return;
    }

    data->encounterMove->decreasePP();
    // TODO: Move animation
    auto category = data->encounterMove->getDefinition()->getCategory();
    if (category != MoveDef::STATUS)
    {
        // TODO: Damage animation
        data->playerPkmn->decreaseHP(
            SingleBattleScene::computeDamage(data->opponentPkmn, data->playerPkmn, data->encounterMove, data->weather));

        if (data->playerPkmn->isKO())
        {
            bool hasNextPkmn = false;
            for (auto const& pkmn : Game::instance()->data.player.pkmns)
            {
                if (pkmn && !pkmn->isKO())
                {
                    hasNextPkmn = true;
                    break;
                }
            }

            if (hasNextPkmn)
            {
                pkmnFaintSpeech = std::make_unique<TextSpeech>(renderer);
                boost::format pkmnFaintText =
                    boost::format(lc::translate("%1% fainted !")) % data->playerPkmn->getDisplayName();
                pkmnFaintSpeech->setTexts({pkmnFaintText.str()});
                pkmnFaintSpeech->start();
            }
            else
            {
                endSpeech = std::make_unique<TextSpeech>(renderer);
                endSpeech->setTexts({lc::translate("The battle is over ! You lost !")});
                endSpeech->start();
            }

            data->playerPkmn = nullptr;

            return;
        }
    }

    if (data->playerFirst)
        data->state = WEATHER;
    else
        data->state = PLAYER_MOVES;
}

void SingleBattleOpponentMovesPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (pkmnFaintSpeech)
    {
        pkmnFaintSpeech->draw(fps, rs);
        return;
    }

    if (opponentMoveSpeech)
    {
        opponentMoveSpeech->draw(fps, rs);
        return;
    }

    if (endSpeech)
        endSpeech->draw(fps, rs);
}
