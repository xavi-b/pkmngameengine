#ifndef SINGLEBATTLEEXPERIENCEPHASE_H
#define SINGLEBATTLEEXPERIENCEPHASE_H

#include "singlebattlephase.h"

#include <string>
#include <vector>

class SingleBattleExperiencePhase : public SingleBattlePhase
{
public:
    SingleBattleExperiencePhase(SingleBattleScene&                     scene,
                                std::shared_ptr<SingleBattleSceneData> data,
                                SDL_Renderer*                          renderer);

    void showSpeech(std::vector<std::string> const& texts);
    void showEndSpeech(std::string const& text);
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool onExperienceResolvedNextPkmn() = 0;

private:
    std::unique_ptr<TextSpeech> experienceSpeech;
    std::unique_ptr<TextSpeech> moveToLearnSpeech;
    std::unique_ptr<TextSpeech> endSpeech;
};

#endif // SINGLEBATTLEEXPERIENCEPHASE_H
