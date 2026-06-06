#include "singlebattleweatherphase.h"

#include "../singlebattlescene.h"
#include "animations/weather/blizzardanimation.h"
#include "animations/weather/harshsunlightanimation.h"
#include "animations/weather/rainanimation.h"
#include "animations/weather/sandstormanimation.h"
#include "animations/weather/weatheranimation.h"
#include "textspeech.h"

SingleBattleWeatherPhase::SingleBattleWeatherPhase(SingleBattleScene&                     scene,
                                                   std::shared_ptr<SingleBattleSceneData> data,
                                                   SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleWeatherPhase::setWeather(Map::Weather weather)
{
    data->weather = weather;

    if (weather == Map::Weather::NONE)
    {
        weatherAnimation.release();
        weatherSpeech.release();
        return;
    }

    weatherSpeech = std::make_unique<TextSpeech>(renderer);

    switch (weather)
    {
    case Map::HARSH_SUNLIGHT:
        weatherAnimation = std::make_unique<HarshSunlightAnimation>(renderer, false);
        break;
    case Map::HAIL:
        weatherAnimation = std::make_unique<BlizzardAnimation>(renderer, false);
        break;
    case Map::SANDSTORM:
        weatherAnimation = std::make_unique<SandstormAnimation>(renderer, false);
        break;
    case Map::RAIN:
        weatherAnimation = std::make_unique<RainAnimation>(renderer, false);
        break;
    default:
        break;
    }
    weatherSpeech->setTexts({weatherAnimation->getStartingText()});
    weatherSpeech->start();
}

void SingleBattleWeatherPhase::update(Inputs const* inputs)
{
    if (data->weather == Map::Weather::NONE)
    {
        data->state = ACTIONS;
    }

    if (weatherAnimation)
    {
        if (!weatherAnimation->isStarted())
        {
            weatherAnimation->start();
        }

        if (weatherAnimation->isRunning())
        {
            weatherAnimation->incrementTicks();
        }

        weatherSpeech->update(inputs);

        if (weatherSpeech->shouldClose())
        {
            weatherSpeech->setTexts({weatherAnimation->getContinuingText()});
            weatherSpeech->reset();
            weatherSpeech->start();
            weatherAnimation->reset();

            data->state = ACTIONS;
        }
    }
}

void SingleBattleWeatherPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (weatherAnimation)
    {
        if (weatherAnimation->isRunning())
        {
            weatherAnimation->draw(fps, rs);
        }

        if (!weatherSpeech->shouldClose())
        {
            weatherSpeech->draw(fps, rs);
        }
    }
}
