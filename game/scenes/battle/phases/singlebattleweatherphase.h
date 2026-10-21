#ifndef SINGLEBATTLEWEATHERPHASE_H
#define SINGLEBATTLEWEATHERPHASE_H

#include "map.h"
#include "singlebattlephase.h"
#include "textspeech.h"

#include <vector>

class SingleBattleWeatherPhase : public SingleBattlePhase
{
public:
    SingleBattleWeatherPhase(SingleBattleScene&                     scene,
                             std::shared_ptr<SingleBattleSceneData> data,
                             SDL_Renderer*                          renderer);

    void setWeather(Map::Weather weather);
    void preupdate() override;
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    enum class Step
    {
        Weather,
        Residuals,
        Advance
    };

    void showSpeech(std::vector<std::string> const& texts);
    void buildResidualMessages();
    void finishTurn();

    Step                        step = Step::Weather;
    std::unique_ptr<TextSpeech> weatherSpeech;
    std::unique_ptr<TextSpeech>       residualSpeech;
    std::vector<std::string>          residualMessages;
    bool                              residualsApplied          = false;
    bool                              playerFaintedFromResidual   = false;
    bool                              opponentFaintedFromResidual = false;
};

#endif // SINGLEBATTLEWEATHERPHASE_H
