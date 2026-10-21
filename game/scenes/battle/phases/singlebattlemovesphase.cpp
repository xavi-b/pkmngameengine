#include "singlebattlemovesphase.h"

#include "../moveeffects.h"
#include "../singlebattlescene.h"
#include "../singlebattlescenedata.h"
#include "../statuscondition.h"
#include "battles/battleactions.h"
#include "game.h"
#include "itemutils.h"
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
    moveSelection->setBattlers(&data->playerBattler, data->opponentPkmn, &data->opponentBattler);
    moveSelection->setGravity(false);
    clearEncoreIfMoveDepleted(data->playerPkmn, data->playerBattler);
    clearEncoreIfMoveDepleted(data->opponentPkmn, data->opponentBattler);
    data->playerBattler.movedThisTurn        = false;
    data->playerBattler.tookDamageThisTurn   = false;
    data->playerBattler.statsLoweredThisTurn = false;
    data->opponentBattler.movedThisTurn        = false;
    data->opponentBattler.tookDamageThisTurn   = false;
    data->opponentBattler.statsLoweredThisTurn = false;
}

void SingleBattleMovesPhase::update(Inputs const* inputs)
{
    if (scene.consumeEjectSwitch())
        return;
    if (noPpLeftSpeech)
    {
        inputs->setHandler(noPpLeftSpeech->inputId());
        noPpLeftSpeech->update(inputs);
        if (noPpLeftSpeech->shouldClose())
            noPpLeftSpeech.release();
        return;
    }

    if (moveSelection->shouldQuit())
    {
        data->state = ACTIONS;
    }
    else if (!moveSelection->anyMoveUsable() || moveSelection->isSelected())
    {
        bool struggle = !moveSelection->anyMoveUsable();
        if (struggle)
            data->playerMove = struggleMove();
        else
            data->playerMove = data->playerPkmn->getMoves().at(moveSelection->selectedIndex());

        auto reject = [&](char const* text) {
            noPpLeftSpeech = std::make_unique<TextSpeech>(renderer);
            noPpLeftSpeech->setTexts({lc::translate(text)});
            noPpLeftSpeech->start();
            moveSelection->reset();
        };

        if (!struggle && data->playerMove && data->playerMove->getDefinition()
            && data->playerBattler.multiTurnLock > 0 && !data->playerBattler.rolloutMoveId.empty()
            && data->playerBattler.rolloutMoveId != data->playerMove->getDefinition()->getId())
            reject("The rollout continues!");
        else if (!struggle && data->playerMove && data->playerMove->getDefinition()
            && data->playerBattler.multiTurnLock > 0 && !data->playerBattler.rampageMoveId.empty()
            && data->playerBattler.rampageMoveId != data->playerMove->getDefinition()->getId())
            reject("The rampage continues!");
        else if (!struggle && data->playerMove && data->playerMove->getDefinition()
            && data->playerBattler.bideActive
            && data->playerMove->getDefinition()->getFunctionCode()
                   != MoveDef::FunctionCode::MultiTurnAttackBideThenReturnDoubleDamage)
            reject("The bide continues!");
        else if (!struggle && data->playerMove && data->playerMove->getDefinition()
            && data->playerBattler.uproarTurns > 0 && !data->playerBattler.uproarMoveId.empty()
            && data->playerBattler.uproarMoveId != data->playerMove->getDefinition()->getId())
            reject("The uproar continues!");
        else if (!struggle && data->playerMove && data->playerMove->getDefinition()
            && data->playerBattler.encoreTurns > 0 && !data->playerBattler.encoreMoveId.empty()
            && data->playerBattler.encoreMoveId != data->playerMove->getDefinition()->getId())
            reject("The encore forces another move!");
        else if (!struggle && data->playerMove && data->playerMove->getDefinition()
            && data->playerBattler.disableTurns > 0
            && data->playerBattler.disabledMoveId == data->playerMove->getDefinition()->getId())
            reject("That move is disabled!");
        else if (struggle || (data->playerMove && data->playerMove->getCurrentPP() > 0))
        {
            if (!data->opponentActionLocked)
            {
                data->chooseOpponentAction();
                data->opponentActionLocked = true;
            }

            if (data->opponentAction == BattleActions::Type::MOVES)
            {
                if (!data->encounterMove)
                    data->playerFirst = true;
                else if (movePriority(data->playerMove->getDefinition(), data->playerPkmn, data->playerBattler, *data)
                         > movePriority(data->encounterMove->getDefinition(), data->opponentPkmn, data->opponentBattler, *data))
                    data->playerFirst = true;
                else if (movePriority(data->playerMove->getDefinition(), data->playerPkmn, data->playerBattler, *data)
                         < movePriority(data->encounterMove->getDefinition(), data->opponentPkmn, data->opponentBattler, *data))
                    data->playerFirst = false;
                else
                {
                    auto holds = [](Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler, char const* id) {
                        return ItemUtils::activeHeldId(pkmn, &battler) == id;
                    };
                    auto quickBracket = [&](Pkmn::PkmnPtr const& mon, BattleBattler& battler, Move::MovePtr const& move) {
                        bool custap         = battler.custapBerry;
                        battler.custapBerry = false;
                        bool claw           = holds(mon, battler, "QUICKCLAW") && Utils::randuint(1, 100) <= 20;
                        bool draw           = false;
                        if (!battler.abilitySuppressed && battler.abilityId == "QUICKDRAW" && Utils::randuint(1, 100) <= 30
                            && move && move->getDefinition())
                        {
                            auto def = move->getDefinition();
                            draw     = def->getCategory() != MoveDef::Category::STATUS && def->getPower() > 0;
                        }
                        return custap || claw || draw;
                    };
                    bool playerQuick = quickBracket(data->playerPkmn, data->playerBattler, data->playerMove);
                    bool foeQuick    = quickBracket(data->opponentPkmn, data->opponentBattler, data->encounterMove);
                    if (playerQuick != foeQuick)
                        data->playerFirst = playerQuick;
                    else
                    {
                    size_t playerSpeed    = battleSpeed(data->playerPkmn, &data->playerBattler, data->fieldWeather(), false);
                    size_t encounterSpeed = battleSpeed(data->opponentPkmn, &data->opponentBattler, data->fieldWeather(), false);
                        auto lags = [](Pkmn::PkmnPtr const& mon, BattleBattler const& battler) {
                            if (!battler.abilitySuppressed && battler.abilityId == "STALL")
                                return true;
                            return ItemUtils::activeHeldId(mon, &battler) == "LAGGINGTAIL"
                                || ItemUtils::activeHeldId(mon, &battler) == "FULLINCENSE";
                        };
                        bool playerLag = lags(data->playerPkmn, data->playerBattler);
                        bool foeLag    = lags(data->opponentPkmn, data->opponentBattler);
                        if (playerLag != foeLag)
                            data->playerFirst = foeLag;
                        else
                        {
                        if (playerSpeed > encounterSpeed)
                            data->playerFirst = true;
                        else if (playerSpeed < encounterSpeed)
                            data->playerFirst = false;
                        else
                            data->playerFirst = Utils::randuint(0, 1);
                        }
                    }
                }
            }
            else
            {
                data->playerFirst = false;
            }

            armSelectSetupMove(data->playerBattler, data->playerMove);
            if (data->opponentAction == BattleActions::Type::MOVES)
                armSelectSetupMove(data->opponentBattler, data->encounterMove);

            std::vector<std::string> announces;
            appendSelectAnnounce(announces, data->playerPkmn, data->playerMove);
            if (data->opponentAction == BattleActions::Type::MOVES)
                appendSelectAnnounce(announces, data->opponentPkmn, data->encounterMove);
            if (!announces.empty())
                data->pendingAbilityMessages.insert(data->pendingAbilityMessages.begin(), announces.begin(),
                                                    announces.end());

            // Quash: waiting side must act last among remaining actions.
            if (data->playerMustMoveLast && !data->playerBattler.movedThisTurn)
                data->playerFirst = false;
            if (data->opponentMustMoveLast && !data->opponentBattler.movedThisTurn)
                data->playerFirst = true;

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
        inputs->setHandler(moveSelection->inputId());
        moveSelection->update(inputs);
    }
}

void SingleBattleMovesPhase::draw(Fps const* fps, RenderSizes rs)
{
    moveSelection->draw(fps, rs);
    if (noPpLeftSpeech)
        noPpLeftSpeech->draw(fps, rs);
}
