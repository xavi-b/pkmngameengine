#ifndef SINGLEBATTLEWEATHERPHASE_H
#define SINGLEBATTLEWEATHERPHASE_H

#include "animations/weather/weatheranimation.h"
#include "map.h"
#include "singlebattlephase.h"

class SingleBattleWeatherPhase : public SingleBattlePhase
{
public:
    SingleBattleWeatherPhase(SingleBattleScene&                     scene,
                             std::shared_ptr<SingleBattleSceneData> data,
                             SDL_Renderer*                          renderer);

    void setWeather(Map::Weather weather);
    void update(Inputs const* inputs) override;
    void draw(Fps const* fps, RenderSizes rs) override;

private:
    std::unique_ptr<WeatherAnimation> weatherAnimation;
    std::unique_ptr<TextSpeech>       weatherSpeech;
};

#endif // SINGLEBATTLEWEATHERPHASE_H
