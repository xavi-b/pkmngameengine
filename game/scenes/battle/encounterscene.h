#ifndef ENCOUNTERSCENE_H
#define ENCOUNTERSCENE_H

#include "singlebattlescene.h"

class EncounterScene : public SingleBattleScene
{
public:
    EncounterScene(SDL_Renderer* renderer);
    virtual ~EncounterScene();

    virtual std::string name() override;
    void                setEncounterPkmn(Pkmn::PkmnPtr const& newEncounterPkmn);

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
};

#endif // ENCOUNTERSCENE_H
