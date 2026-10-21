#include "trainerexperiencephase.h"

#include "../../abilitybattle.h"
#include "../../singlebattlescene.h"
#include "../../trainerscenedata.h"
#include "game.h"
#include "utils.h"

#include <boost/format.hpp>
#include <vector>

TrainerExperiencePhase::TrainerExperiencePhase(SingleBattleScene&                     scene,
                                               std::shared_ptr<SingleBattleSceneData> data,
                                               SDL_Renderer*                          renderer)
    : SingleBattleExperiencePhase(scene, data, renderer)
{
}

std::vector<std::string> TrainerExperiencePhase::victoryTexts()
{
    std::vector<std::string> texts = {lc::translate("The battle is over ! You won !")};

    auto trainerData = std::static_pointer_cast<TrainerSceneData>(data);
    if (trainerData->opponentTrainer && trainerData->opponentTrainer->money > 0)
    {
        size_t prize = trainerData->opponentTrainer->money;
        bool doubled = trainerData->doublePrize;
        if (!doubled)
        {
            for (auto const& mon : trainerData->participatingPlayerPkmns)
            {
                if (!mon || !mon->getHeldItem() || !mon->getHeldItem()->getDefinition())
                    continue;
                auto const& itemId = mon->getHeldItem()->getDefinition()->getId();
                if (itemId == "AMULETCOIN" || itemId == "LUCKINCENSE")
                {
                    doubled = true;
                    break;
                }
            }
        }
        if (doubled)
            prize *= 2;
        Game::instance()->data.player.money += prize;
        boost::format prizeText =
            boost::format(lc::translate("You got $%1% for winning !")) % prize;
        texts.push_back(prizeText.str());
    }

    return texts;
}

bool TrainerExperiencePhase::onExperienceResolvedNextPkmn()
{
    auto trainerData = std::static_pointer_cast<TrainerSceneData>(data);

    if (!trainerData->shouldSwitchOpponentPkmn)
        return false;

    if (!trainerData->opponentSwitchAnnounced)
    {
        if (!trainerData->nextOpponentPkmn)
            return false;

        std::vector<std::string> texts;
        if (!trainerData->defeatedOpponentPkmnName.empty())
        {
            boost::format faintText =
                boost::format(lc::translate("Foe %1% fainted !")) % trainerData->defeatedOpponentPkmnName;
            texts.push_back(faintText.str());
        }

        if (trainerData->opponentTrainer && !trainerData->opponentTrainer->name.empty())
        {
            boost::format sendOutText = boost::format(lc::translate("Trainer %1% sent out %2% !"))
                                      % trainerData->opponentTrainer->name
                                      % predictedIllusionName(trainerData->nextOpponentPkmn, false, *data);
            texts.push_back(sendOutText.str());
        }
        else
        {
            boost::format sendOutText =
                boost::format(lc::translate("Foe sent out %1% !"))
                % predictedIllusionName(trainerData->nextOpponentPkmn, false, *data);
            texts.push_back(sendOutText.str());
        }

        showSpeech(texts);
        trainerData->opponentSwitchAnnounced = true;
        return true;
    }

    if (!trainerData->nextOpponentPkmn)
        return false;

    data->state = OPPONENT_PKMNS;
    return true;
}
