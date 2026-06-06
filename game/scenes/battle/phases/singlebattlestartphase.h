#ifndef SINGLEBATTLESTARTPHASE_H
#define SINGLEBATTLESTARTPHASE_H

#include "animations/eyeanimation.h"
#include "singlebattlephase.h"

#include <string>

class SingleBattleStartPhase : public SingleBattlePhase
{
public:
    SingleBattleStartPhase(SingleBattleScene&                     scene,
                           std::shared_ptr<SingleBattleSceneData> data,
                           SDL_Renderer*                          renderer);

    void init(std::string const& encounterText, std::string const& firstPkmnText);
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::unique_ptr<EyeAnimation> eyeAnimation;
    std::unique_ptr<TextSpeech>   pkmnEncounterSpeech;
    std::unique_ptr<TextSpeech>   firstPkmnSpeech;
};

#endif // SINGLEBATTLESTARTPHASE_H
