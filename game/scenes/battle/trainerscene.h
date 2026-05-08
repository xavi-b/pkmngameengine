#ifndef TRAINERSCENE_H
#define TRAINERSCENE_H

#include "singlebattlescene.h"

class TrainerScene : public SingleBattleScene
{
public:
    TrainerScene(SDL_Renderer* renderer);
    virtual ~TrainerScene();

    virtual std::string name() override;

protected:
    virtual std::string encounterStartText() const override;
    virtual std::string opponentMoveText(Move::MovePtr const& move) const override;
    virtual bool        canCaptureOpponent() const override;
    virtual bool        canPlayerRun() const override;
    virtual bool        tryPlayerRun() override;
    virtual float       battleExperienceMultiplier() const override;
};

#endif // TRAINERSCENE_H
