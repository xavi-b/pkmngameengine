#include "trainerexperiencephase.h"

#include "../../singlebattlescene.h"
#include "../../trainerscenedata.h"
#include "utils.h"

#include <boost/format.hpp>
#include <vector>

TrainerExperiencePhase::TrainerExperiencePhase(SingleBattleScene&                     scene,
                                               std::shared_ptr<SingleBattleSceneData> data,
                                               SDL_Renderer*                          renderer)
    : SingleBattleExperiencePhase(scene, data, renderer)
{
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
                                      % trainerData->nextOpponentPkmn->getDisplayName();
            texts.push_back(sendOutText.str());
        }
        else
        {
            boost::format sendOutText =
                boost::format(lc::translate("Foe sent out %1% !")) % trainerData->nextOpponentPkmn->getDisplayName();
            texts.push_back(sendOutText.str());
        }

        showSpeech(texts);
        trainerData->opponentSwitchAnnounced = true;
        return true;
    }

    trainerData->shouldSwitchOpponentPkmn = false;
    if (!trainerData->nextOpponentPkmn)
        return false;

    scene.setOpponentPkmn(trainerData->nextOpponentPkmn);
    trainerData->nextOpponentPkmn.reset();
    trainerData->defeatedOpponentPkmnName.clear();
    trainerData->opponentSwitchAnnounced = false;
    data->state                          = WEATHER;
    return true;
}
