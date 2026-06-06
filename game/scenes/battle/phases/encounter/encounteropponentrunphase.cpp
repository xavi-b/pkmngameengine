#include "encounteropponentrunphase.h"

#include "utils.h"

#include <boost/format.hpp>

EncounterOpponentRunPhase::EncounterOpponentRunPhase(SingleBattleScene&                     scene,
                                                     std::shared_ptr<SingleBattleSceneData> data,
                                                     SDL_Renderer*                          renderer)
    : SingleBattleOpponentRunPhase(scene, data, renderer)
{
}

std::string EncounterOpponentRunPhase::opponentRunText() const
{
    boost::format runText = boost::format(lc::translate("Wild %1% fled !"))
                          % (data->opponentPkmn ? data->opponentPkmn->getDisplayName() : "#ERROR");
    return runText.str();
}
