#ifndef TRAINERSCENE_H
#define TRAINERSCENE_H

#include "entities/trainer.h"
#include "singlebattlescene.h"
#include "trainerscenedata.h"

class TrainerScene : public SingleBattleScene
{
public:
    TrainerScene(SDL_Renderer* renderer);
    virtual ~TrainerScene();

    virtual std::string name() override;
    void                setOpponentTrainer(Trainer::TrainerPtr const& newOpponentTrainer);
    virtual void        init() override;

protected:
    virtual std::string encounterStartText() const override;

    virtual void initPhases() override;

private:
    Pkmn::PkmnPtr nextUsableOpponentPkmn() const;
};

#endif // TRAINERSCENE_H
