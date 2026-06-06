#ifndef SINGLEBATTLEOPPONENTMOVESPHASE_H
#define SINGLEBATTLEOPPONENTMOVESPHASE_H

#include "pkmn.h"
#include "singlebattlephase.h"

#include <string>

class SingleBattleOpponentMovesPhase : public SingleBattlePhase
{
public:
    SingleBattleOpponentMovesPhase(SingleBattleScene&                     scene,
                                   std::shared_ptr<SingleBattleSceneData> data,
                                   SDL_Renderer*                          renderer);

    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string opponentMoveText(Move::MovePtr const& move) const = 0;

private:
    std::unique_ptr<TextSpeech> opponentMoveSpeech;
    std::unique_ptr<TextSpeech> pkmnFaintSpeech;
    std::unique_ptr<TextSpeech> endSpeech;
};

#endif // SINGLEBATTLEOPPONENTMOVESPHASE_H
