#ifndef SINGLEBATTLEENDPHASE_H
#define SINGLEBATTLEENDPHASE_H

#include "animations/fadeanimation.h"
#include "singlebattlephase.h"

class SingleBattleEndPhase : public SingleBattlePhase
{
public:
    SingleBattleEndPhase(SingleBattleScene& scene, std::shared_ptr<SingleBattleSceneData> data, SDL_Renderer* renderer);

    bool isFinished() const;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::unique_ptr<FadeAnimation> fadeOutAnimation;
};

#endif // SINGLEBATTLEENDPHASE_H
