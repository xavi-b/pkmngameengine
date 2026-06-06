#ifndef TRAINERSCENEDATA_H
#define TRAINERSCENEDATA_H

#include "entities/trainer.h"
#include "pkmn.h"
#include "singlebattlescenedata.h"

struct TrainerSceneData : public SingleBattleSceneData
{
    Trainer::TrainerPtr opponentTrainer;
    bool                shouldSwitchOpponentPkmn = false;
    bool                opponentSwitchAnnounced  = false;
    std::string         defeatedOpponentPkmnName;
    Pkmn::PkmnPtr       nextOpponentPkmn = nullptr;

    virtual void chooseOpponentAction() override;
};

#endif // TRAINERSCENEDATA_H
