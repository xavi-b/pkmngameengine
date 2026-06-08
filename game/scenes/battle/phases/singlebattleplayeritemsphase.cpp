#include "singlebattleplayeritemsphase.h"

#include "../singlebattlescene.h"
#include "game.h"
#include "itemutils.h"
#include "textspeech.h"
#include "utils.h"

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
        itemUseSpeech->update(inputs);
        if (!itemUseSpeech->shouldClose())
            return;

        itemUseSpeech.release();

        if (!data->itemUseResultUsed)
        {
            data->state = ACTIONS;
            return;
        }

        if (data->itemUseResultCaptureSuccess)
        {
            data->state = END;
            return;
        }

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
        return;
    }

    bool selected = data->selectedItem != nullptr;
    if (selected)
    {
        bool                     used           = false;
        bool                     captureSuccess = false;
        std::vector<std::string> itemMessages;
        auto                     def = data->selectedItem->getDefinition();
        if (def)
        {
            std::string itemName = def->getName();
            switch (def->getBattleUse())
            {
            case ItemDef::BattleUse::OnPkmn: {
                auto target = data->itemTargetPkmn ? data->itemTargetPkmn : data->playerPkmn;
                used        = ItemUtils::useItemOnPkmn(data->selectedItem, target, true);
                data->itemTargetPkmn.reset();
                if (used && target)
                {
                    boost::format itemText = boost::format(lc::translate("%1% used %2% on %3% !"))
                                           % Game::instance()->data.player.name % itemName % target->getDisplayName();
                    itemMessages.push_back(itemText.str());
                }
                break;
            }
            case ItemDef::BattleUse::OnMove:
                // TODO: Implement
                used = false;
                break;
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
                captureSuccess = ItemUtils::useCaptureBall(data->selectedItem, data->opponentPkmn);
                if (captureSuccess)
                    ItemUtils::addPkmnToPlayer(data->opponentPkmn);
                if (captureSuccess)
                    itemMessages.push_back(lc::translate("Gotcha ! The Pkmn was caught !"));
                else
                    itemMessages.push_back(lc::translate("Oh no ! The Pkmn broke free !"));
                break;
            }
            case ItemDef::BattleUse::Direct:
            case ItemDef::BattleUse::OnBattler:
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
                itemUseSpeech                     = std::make_unique<TextSpeech>(renderer);
                itemUseSpeech->setTexts(itemMessages);
                itemUseSpeech->start();
                return;
            }

            data->selectedItem.reset();
            data->state = ACTIONS;
            return;
        }

        Game::instance()->data.player.removeOneItem(data->selectedItem->getDefinition());
        data->selectedItem.reset();
        data->itemUseResultUsed           = used;
        data->itemUseResultCaptureSuccess = captureSuccess;
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
