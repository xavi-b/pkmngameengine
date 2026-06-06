#ifndef SINGLEBATTLEPLAYERRUNPHASE_H
#define SINGLEBATTLEPLAYERRUNPHASE_H

#include "singlebattlephase.h"

class SingleBattlePlayerRunPhase : public SingleBattlePhase
{
public:
    SingleBattlePlayerRunPhase(SingleBattleScene&                     scene,
                               std::shared_ptr<SingleBattleSceneData> data,
                               SDL_Renderer*                          renderer);

    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool canPlayerRun() const = 0;
    virtual bool tryPlayerRun()       = 0;

private:
    std::unique_ptr<TextSpeech> runSpeech;
    std::unique_ptr<TextSpeech> failedRunSpeech;
};

#endif // SINGLEBATTLEPLAYERRUNPHASE_H
