#include "trainerscene.h"

#include "utils.h"

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

std::string TrainerScene::encounterStartText() const
{
    return lc::translate("A trainer wants to battle !");
}

std::string TrainerScene::opponentMoveText(Move::MovePtr const& move) const
{
    boost::format moveText = boost::format(lc::translate("Foe %1% uses %2% !"))
                           % (encounterPkmn ? encounterPkmn->getDisplayName() : "#ERROR")
                           % (move && move->getDefinition() ? move->getDefinition()->getName() : "#ERROR");
    return moveText.str();
}

bool TrainerScene::canCaptureOpponent() const
{
    return false;
}

bool TrainerScene::canPlayerRun() const
{
    return false;
}

bool TrainerScene::tryPlayerRun()
{
    return false;
}

float TrainerScene::battleExperienceMultiplier() const
{
    return 1.5f;
}