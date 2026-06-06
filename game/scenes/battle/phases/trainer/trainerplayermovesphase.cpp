#include "trainerplayermovesphase.h"

#include "../../trainerscenedata.h"

TrainerPlayerMovesPhase::TrainerPlayerMovesPhase(SingleBattleScene&                     scene,
                                                 std::shared_ptr<SingleBattleSceneData> data,
                                                 SDL_Renderer*                          renderer)
    : SingleBattlePlayerMovesPhase(scene, data, renderer)
{
}

namespace
{
Pkmn::PkmnPtr nextUsableOpponentPkmn(std::shared_ptr<TrainerSceneData> const& trainerData)
{
    if (!trainerData->opponentTrainer)
        return nullptr;

    for (auto const& pkmn : trainerData->opponentTrainer->pkmns)
    {
        if (pkmn && !pkmn->isKO() && pkmn != trainerData->opponentPkmn)
            return pkmn;
    }

    return nullptr;
}
} // namespace

float TrainerPlayerMovesPhase::battleExperienceMultiplier() const
{
    return 1.5f;
}

void TrainerPlayerMovesPhase::onOpponentPkmnDefeated()
{
    auto trainerData = std::static_pointer_cast<TrainerSceneData>(data);

    trainerData->defeatedOpponentPkmnName =
        trainerData->opponentPkmn ? trainerData->opponentPkmn->getDisplayName() : "";
    trainerData->nextOpponentPkmn         = nextUsableOpponentPkmn(trainerData);
    trainerData->shouldSwitchOpponentPkmn = trainerData->nextOpponentPkmn != nullptr;
    trainerData->opponentSwitchAnnounced  = false;
}
