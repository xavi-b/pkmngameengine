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

    firstPkmnSpeech = std::make_unique<TextSpeech>(renderer);
    firstPkmnSpeech->setTexts({firstPkmnText});

    battleUi()->startIntro(foeSentFromBall());
}

void SingleBattleStartPhase::update(Inputs const* inputs)
{
    if (!eyeAnimation->isFinished())
        eyeAnimation->incrementTicks();

    if (!battleUi()->isFoeEntranceDone())
    {
        battleUi()->incrementTicks();
        return;
    }

    if (pkmnEncounterSpeech)
    {
        if (!pkmnEncounterSpeech->isStarted())
            pkmnEncounterSpeech->start();

        inputs->setHandler(pkmnEncounterSpeech->inputId());
        pkmnEncounterSpeech->update(inputs);
        if (pkmnEncounterSpeech->shouldClose())
            pkmnEncounterSpeech.reset();
        return;
    }

    if (!battleUi()->isPlayerSendOutDone())
    {
        if (!battleUi()->isPlayerSendOutStarted())
            battleUi()->startPlayerSendOut();
        else
            battleUi()->incrementTicks();
        return;
    }

    if (firstPkmnSpeech)
    {
        if (!firstPkmnSpeech->isStarted())
            firstPkmnSpeech->start();

        inputs->setHandler(firstPkmnSpeech->inputId());
        firstPkmnSpeech->update(inputs);
        if (firstPkmnSpeech->shouldClose())
        {
            firstPkmnSpeech.reset();
            data->state = WEATHER;
        }
    }
}

void SingleBattleStartPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (pkmnEncounterSpeech && pkmnEncounterSpeech->isStarted() && battleUi()->isFoeEntranceDone())
        pkmnEncounterSpeech->draw(fps, rs);
    else if (firstPkmnSpeech && firstPkmnSpeech->isStarted() && battleUi()->isPlayerSendOutDone())
        firstPkmnSpeech->draw(fps, rs);

    if (eyeAnimation->isRunning())
        eyeAnimation->draw(fps, rs);
}
