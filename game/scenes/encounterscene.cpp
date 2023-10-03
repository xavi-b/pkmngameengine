#include "encounterscene.h"

#include "game.h"
#include "utils.h"

EncounterScene::EncounterScene(SDL_Renderer* renderer) : BattleScene(renderer)
{
}

EncounterScene::~EncounterScene()
{
}

void EncounterScene::draw(Fps const* fps, RenderSizes rs)
{
    battleBackground->draw(fps, rs);

    if (state == MENU)
    {
        battleSpeech->draw(fps, rs);
        if (battleSpeech->mayClose())
        {
            battleActions->draw(fps, rs);
        }
    }
    else if (state == MOVES)
    {
        moveSelection->draw(fps, rs);
    }
}

std::string EncounterScene::name()
{
    return "EncounterScene";
}

void EncounterScene::setEncounteredPkmn(Pkmn::PkmnPtr const& newEncounteredPkmn)
{
    encounteredPkmn = newEncounteredPkmn;
}
