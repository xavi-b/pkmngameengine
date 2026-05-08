#include "encounterscene.h"

EncounterScene::EncounterScene(SDL_Renderer* renderer) : SingleBattleScene(renderer)
{

}

EncounterScene::~EncounterScene()
{
}

std::string EncounterScene::name()
{
    return "EncounterScene";
}