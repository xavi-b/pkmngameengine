#include "traineropponentmovesphase.h"

#include "utils.h"

#include <boost/format.hpp>

TrainerOpponentMovesPhase::TrainerOpponentMovesPhase(SingleBattleScene&                     scene,
                                                     std::shared_ptr<SingleBattleSceneData> data,
                                                     SDL_Renderer*                          renderer)
    : SingleBattleOpponentMovesPhase(scene, data, renderer)
{
}

std::string TrainerOpponentMovesPhase::opponentMoveText(Move::MovePtr const& move) const
{
    boost::format moveText = boost::format(lc::translate("Foe %1% uses %2% !"))
                           % (data->opponentPkmn ? data->opponentPkmn->getDisplayName() : "#ERROR")
                           % (move && move->getDefinition() ? move->getDefinition()->getName() : "#ERROR");
    return moveText.str();
}
