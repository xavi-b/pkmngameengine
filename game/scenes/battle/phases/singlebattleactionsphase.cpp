#include "singlebattleactionsphase.h"

#include "../moveeffects.h"
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
    auto endRoost = [](BattleBattler& battler) {
        if (!battler.roosting)
            return;
        battler.typesOverride = battler.roostTypes;
        battler.roostTypes.clear();
        battler.roosting = false;
    };
    endRoost(data->playerBattler);
    endRoost(data->opponentBattler);
    data->playerBattler.statsRaisedThisTurn   = false;
    data->opponentBattler.statsRaisedThisTurn = false;
    data->playerRetaliate         = data->playerFaintedThisTurn;
    data->opponentRetaliate       = data->opponentFaintedThisTurn;
    data->playerFaintedThisTurn   = false;
    data->opponentFaintedThisTurn = false;
    data->playerMustMoveLast      = false;
    data->opponentMustMoveLast    = false;
    data->playerSwitchQueued      = false;
    data->playerItemQueued        = false;
    data->opponentActionLocked    = false;
    data->opponentSwitchDone      = false;
    data->opponentItemDone        = false;
    data->playerEchoedThisTurn    = false;
    data->opponentEchoedThisTurn  = false;
    data->pendingPlayerReplace    = false;
    data->replaceAfterResidual    = false;
}

void SingleBattleActionsPhase::update(Inputs const* inputs)
{
    if (scene.consumeEjectSwitch())
        return;
    if (battleSpeech->mayClose() && !battleActions->isFinished())
        inputs->setHandler(battleActions->inputId());
    else
        inputs->setHandler(battleSpeech->inputId());

    battleSpeech->update(inputs);
    if (battleSpeech->mayClose())
    {
        if (battleActions->isFinished())
        {
            switch (battleActions->selectedAction())
            {
            case BattleActions::MOVES: {
                data->playerAction = BattleActions::Type::MOVES;
                data->state = MOVES;
                break;
            }
            case BattleActions::BAG:
                data->playerAction = BattleActions::Type::BAG;
                data->state = BAG;
                break;
            case BattleActions::PKMNS:
                // Lock the foe's command at select so Pursuit uses that move, not a re-roll.
                data->playerAction = BattleActions::Type::PKMNS;
                data->chooseOpponentAction();
                data->opponentActionLocked    = true;
                data->playerVoluntarySwitch   = true;
                data->playerSwitchQueued      = true;
                if (data->opponentAction == BattleActions::MOVES)
                {
                    armSelectSetupMove(data->opponentBattler, data->encounterMove);
                    std::vector<std::string> announces;
                    appendSelectAnnounce(announces, data->opponentPkmn, data->encounterMove);
                    data->pendingAbilityMessages.insert(data->pendingAbilityMessages.end(), announces.begin(),
                                                        announces.end());
                }
                data->state = PKMNS;
                break;
            case BattleActions::RUN:
                data->playerAction = BattleActions::Type::RUN;
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
