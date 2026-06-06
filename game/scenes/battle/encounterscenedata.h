#ifndef ENCOUNTERSCENEDATA_H
#define ENCOUNTERSCENEDATA_H

#include "entities/trainer.h"
#include "pkmn.h"
#include "singlebattlescenedata.h"

struct EncounterSceneData : public SingleBattleSceneData
{
    virtual void chooseOpponentAction() override;
};

#endif // ENCOUNTERSCENEDATA_H
