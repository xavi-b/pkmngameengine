#include "trainerscene.h"

TrainerScene::TrainerScene(SDL_Renderer* renderer) : SingleBattleScene(renderer)
{

}

TrainerScene::~TrainerScene()
{
}

std::string TrainerScene::name()
{
    return "TrainerScene";
}