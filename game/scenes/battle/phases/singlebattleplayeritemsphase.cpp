#include "singlebattleplayeritemsphase.h"

#include "../moveeffects.h"
#include "../singlebattlescene.h"
#include "../statuscondition.h"
#include "game.h"
#include "itemutils.h"
#include "textspeech.h"
#include "utils.h"

#include <algorithm>
#include <boost/format.hpp>
#include <vector>

SingleBattlePlayerItemsPhase::SingleBattlePlayerItemsPhase(SingleBattleScene&                     scene,
                                                           std::shared_ptr<SingleBattleSceneData> data,
                                                           SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattlePlayerItemsPhase::update(Inputs const* inputs)
{
    if (itemUseSpeech)
    {
        inputs->setHandler(itemUseSpeech->inputId());
        itemUseSpeech->update(inputs);
        if (!itemUseSpeech->shouldClose())
            return;

        itemUseSpeech.release();

        if (!data->itemUseResultUsed)
        {
            data->playerItemQueued     = false;
            data->opponentActionLocked = false;
            data->state                = ACTIONS;
            return;
        }

        if (data->itemUseResultCaptureSuccess || data->itemUseResultEndsBattle)
        {
            data->state = END;
            return;
        }

        data->playerItemQueued = false;
        if (data->opponentAction == BattleActions::PKMNS && !data->opponentSwitchDone)
        {
            data->state = OPPONENT_PKMNS;
            return;
        }
        switch (data->opponentAction)
        {
        case BattleActions::BAG:
            data->state = data->opponentItemDone ? WEATHER : OPPONENT_ITEMS;
            break;
        case BattleActions::PKMNS:
            data->state = WEATHER;
            break;
        case BattleActions::RUN:
            data->state = OPPONENT_RUN;
            break;
        case BattleActions::MOVES:
        default:
            data->state = OPPONENT_MOVES;
            break;
        }
        return;
    }

    bool selected = data->selectedItem != nullptr;
    if (selected && !data->opponentActionLocked)
    {
        data->chooseOpponentAction();
        data->opponentActionLocked = true;
        data->playerItemQueued     = true;
        if (data->opponentAction == BattleActions::MOVES)
        {
            armSelectSetupMove(data->opponentBattler, data->encounterMove);
            std::vector<std::string> announces;
            appendSelectAnnounce(announces, data->opponentPkmn, data->encounterMove);
            data->pendingAbilityMessages.insert(data->pendingAbilityMessages.end(), announces.begin(),
                                                announces.end());
        }
        if (data->opponentAction == BattleActions::PKMNS && !data->opponentSwitchDone)
        {
            data->state = OPPONENT_PKMNS;
            return;
        }
        if (data->opponentAction == BattleActions::BAG && !data->opponentItemDone && opponentOutspeeds(*data))
        {
            data->state = OPPONENT_ITEMS;
            return;
        }
    }
    if (selected)
    {
        bool                     used           = false;
        bool                     captureSuccess = false;
        bool                     endsBattle     = false;
        std::vector<std::string> itemMessages;
        auto                     def = data->selectedItem->getDefinition();
        if (def)
        {
            std::string itemName = def->getName();
            switch (def->getBattleUse())
            {
            case ItemDef::BattleUse::OnPkmn: {
                auto target = data->itemTargetPkmn ? data->itemTargetPkmn : data->playerPkmn;
                if (target == data->playerPkmn && data->playerBattler.embargoTurns > 0)
                {
                    itemMessages.push_back(lc::translate("Embargo prevents using that item!"));
                    used = false;
                    break;
                }
                used        = ItemUtils::useItemOnPkmn(data->selectedItem, target, true);
                if (ItemUtils::curesConfusion(def->getId()) && target == data->playerPkmn
                    && data->playerConfusion.active)
                {
                    data->playerConfusion.clear();
                    used = true;
                }
                data->itemTargetPkmn.reset();
                if (used && target)
                {
                    boost::format itemText = boost::format(lc::translate("%1% used %2% on %3% !"))
                                           % Game::instance()->data.player.name % itemName % target->getDisplayName();
                    itemMessages.push_back(itemText.str());
                }
                break;
            }
            case ItemDef::BattleUse::OnMove: {
                auto target = data->itemTargetPkmn ? data->itemTargetPkmn : data->playerPkmn;
                if (target == data->playerPkmn && data->playerBattler.embargoTurns > 0)
                {
                    itemMessages.push_back(lc::translate("Embargo prevents using that item!"));
                    used = false;
                    break;
                }
                used        = ItemUtils::useItemOnMove(data->selectedItem, target);
                data->itemTargetPkmn.reset();
                if (used && target)
                {
                    boost::format itemText = boost::format(lc::translate("%1% used %2% on %3% !"))
                                           % Game::instance()->data.player.name % itemName % target->getDisplayName();
                    itemMessages.push_back(itemText.str());
                }
                break;
            }
            case ItemDef::BattleUse::OnFoe: {
                if (!canCaptureOpponent())
                {
                    itemMessages.push_back(lc::translate("You cannot catch this Pkmn !"));
                    used = false;
                    break;
                }

                used = true;
                boost::format throwText =
                    boost::format(lc::translate("%1% threw %2% !")) % Game::instance()->data.player.name % itemName;
                itemMessages.push_back(throwText.str());
                battleUi()->startCapture();
                ItemUtils::CaptureContext capture;
                capture.turn       = std::max(1, data->battleTurn);
                capture.atNight    = data->captureAtNight;
                capture.inCave     = data->captureInCave;
                capture.underwater = data->captureUnderwater;
                capture.fishing    = data->captureFishing;
                if (data->playerPkmn)
                {
                    capture.playerLevel  = data->playerPkmn->getLevel();
                    capture.playerGender = data->playerPkmn->getGender();
                    if (data->playerPkmn->getDefinition())
                        capture.playerSpeciesId = data->playerPkmn->getDefinition()->getId();
                }
                captureSuccess     = ItemUtils::useCaptureBall(data->selectedItem, data->opponentPkmn, capture);
                if (captureSuccess)
                {
                    if (data->opponentPkmn && data->selectedItem && data->selectedItem->getDefinition())
                        data->opponentPkmn->setCaptureBall(data->selectedItem->getDefinition()->getId());
                    ItemUtils::addPkmnToPlayer(data->opponentPkmn);
                }
                if (captureSuccess)
                    itemMessages.push_back(lc::translate("Gotcha ! The Pkmn was caught !"));
                else
                    itemMessages.push_back(lc::translate("Oh no ! The Pkmn broke free !"));
                break;
            }
            case ItemDef::BattleUse::OnBattler: {
                if (!data->playerPkmn)
                    break;
                if (data->playerBattler.embargoTurns > 0)
                {
                    itemMessages.push_back(lc::translate("Embargo prevents using that item!"));
                    used = false;
                    break;
                }
                std::vector<std::string> statLines;
                used = ItemUtils::useItemOnBattler(data->selectedItem,
                                                   data->playerPkmn,
                                                   data->playerBattler,
                                                   &data->playerConfusion,
                                                   *data,
                                                   statLines);
                if (used)
                {
                    boost::format itemText = boost::format(lc::translate("%1% used %2% on %3% !"))
                                           % Game::instance()->data.player.name % itemName
                                           % data->playerPkmn->getDisplayName();
                    itemMessages.push_back(itemText.str());
                    itemMessages.insert(itemMessages.end(), statLines.begin(), statLines.end());
                }
                break;
            }
            case ItemDef::BattleUse::Direct: {
                if (data->playerBattler.embargoTurns > 0)
                {
                    itemMessages.push_back(lc::translate("Embargo prevents using that item!"));
                    used = false;
                    break;
                }
                std::string msg;
                used = ItemUtils::useBattleItemDirect(data->selectedItem,
                                                      data->playerPkmn,
                                                      data->playerBattler,
                                                      endsBattle,
                                                      msg,
                                                      data->playerMist);
                if (used)
                {
                    boost::format itemText =
                        boost::format(lc::translate("%1% used %2% !")) % Game::instance()->data.player.name % itemName;
                    itemMessages.push_back(itemText.str());
                    if (!msg.empty())
                        itemMessages.push_back(msg);
                }
                break;
            }
            case ItemDef::BattleUse::None:
            default:
                used = false;
                break;
            }
        }

        if (!used)
        {
            if (!itemMessages.empty())
            {
                data->selectedItem.reset();
                data->itemUseResultUsed           = false;
                data->itemUseResultCaptureSuccess = false;
                data->itemUseResultEndsBattle     = false;
                itemUseSpeech                     = std::make_unique<TextSpeech>(renderer);
                itemUseSpeech->setTexts(itemMessages);
                itemUseSpeech->start();
                return;
            }

            data->selectedItem.reset();
            data->playerItemQueued     = false;
            data->opponentActionLocked = false;
            data->state                = ACTIONS;
            return;
        }

        if (def && def->isConsumable())
            Game::instance()->data.player.removeOneItem(data->selectedItem->getDefinition());
        data->selectedItem.reset();
        data->itemUseResultUsed           = used;
        data->itemUseResultCaptureSuccess = captureSuccess;
        data->itemUseResultEndsBattle     = endsBattle;
        itemUseSpeech                     = std::make_unique<TextSpeech>(renderer);
        itemUseSpeech->setTexts(itemMessages.empty() ? std::vector<std::string>{lc::translate("Item used !")}
                                                     : itemMessages);
        itemUseSpeech->start();
    }
    else
    {
        data->state = ACTIONS;
    }
}

void SingleBattlePlayerItemsPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (itemUseSpeech)
        itemUseSpeech->draw(fps, rs);
}
