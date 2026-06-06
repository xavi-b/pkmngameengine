#include "singlebattlestartphase.h"

#include "../singlebattlescene.h"
#include "animations/eyeanimation.h"
#include "textspeech.h"

SingleBattleStartPhase::SingleBattleStartPhase(SingleBattleScene&                     scene,
                                               std::shared_ptr<SingleBattleSceneData> data,
                                               SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleStartPhase::init(std::string const& encounterText, std::string const& firstPkmnText)
{
    eyeAnimation = std::make_unique<EyeAnimation>(renderer);
    eyeAnimation->start();

    pkmnEncounterSpeech = std::make_unique<TextSpeech>(renderer);
    pkmnEncounterSpeech->setTexts({encounterText});
    pkmnEncounterSpeech->start();

    firstPkmnSpeech = std::make_unique<TextSpeech>(renderer);
    firstPkmnSpeech->setTexts({firstPkmnText});
    firstPkmnSpeech->start();
}

void SingleBattleStartPhase::update(Inputs const* inputs)
{
    if (!eyeAnimation->isFinished())
    {
        eyeAnimation->incrementTicks();
    }

    // TODO: separate singlebattleui ? foe + player

    // TODO: Open animation
    // Pkmn + player entrance -> singlebattleui
    // Show pkmn databox -> singlebattleui

    if (pkmnEncounterSpeech)
    {
        pkmnEncounterSpeech->update(inputs);
        if (pkmnEncounterSpeech->shouldClose())
        {
            pkmnEncounterSpeech.release();
        }
    }

    // TODO: Pkmn animation
    // Throw player pkmn -> singlebattleui
    // Show player databox -> singlebattleui

    else if (firstPkmnSpeech)
    {
        firstPkmnSpeech->update(inputs);
        if (firstPkmnSpeech->shouldClose())
        {
            firstPkmnSpeech.release();

            data->state = WEATHER;
        }
    }
}

void SingleBattleStartPhase::draw(Fps const* fps, RenderSizes rs)
{
    // TODO: Open animation

    if (pkmnEncounterSpeech)
    {
        pkmnEncounterSpeech->draw(fps, rs);
    }

    // TODO: Pkmn animation

    else if (firstPkmnSpeech)
    {
        firstPkmnSpeech->draw(fps, rs);
    }

    if (eyeAnimation->isRunning())
    {
        eyeAnimation->draw(fps, rs);
    }
}
