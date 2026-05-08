#ifndef TRAINERSCENE_H
#define TRAINERSCENE_H

#include "entities/trainer.h"
#include "singlebattlescene.h"

class TrainerScene : public SingleBattleScene
{
public:
    TrainerScene(SDL_Renderer* renderer);
    virtual ~TrainerScene();

    virtual std::string name() override;
    void                setOpponentTrainer(Trainer::TrainerPtr const& newOpponentTrainer);
    virtual void        init() override;

protected:
    virtual void        chooseOpponentAction() override;
    virtual std::string encounterStartText() const override;
    virtual std::string opponentMoveText(Move::MovePtr const& move) const override;
    virtual std::string opponentRunText() const override;
    virtual bool        canCaptureOpponent() const override;
    virtual bool        canPlayerRun() const override;
    virtual bool        tryPlayerRun() override;
    virtual float       battleExperienceMultiplier() const override;
    virtual void        onOpponentPkmnDefeated() override;
    virtual bool        onExperienceResolvedNextPkmn() override;

private:
    Pkmn::PkmnPtr nextUsableOpponentPkmn() const;

    Trainer::TrainerPtr opponentTrainer;
    bool                shouldSwitchOpponentPkmn = false;
};

#endif // TRAINERSCENE_H
