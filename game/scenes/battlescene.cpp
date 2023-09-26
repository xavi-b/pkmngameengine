#include "battlescene.h"

#include "game.h"
#include "utils.h"

BattleScene::BattleScene(SDL_Renderer* renderer) : Scene(renderer)
{
    battleSpeech = std::make_unique<BattleSpeech>(renderer);
    std::vector<std::string> texts;
    texts.push_back(lc::translate("What should AAAAAAAAAAAA do ?"));
    battleSpeech->setTexts(texts);
    battleSpeech->init();
    battleActions = std::make_unique<BattleActions>(renderer);
    battleActions->init();
}

BattleScene::~BattleScene()
{
}

void BattleScene::init()
{
}

void BattleScene::update(Inputs const* inputs)
{
    if (battleSpeech->mayClose())
    {
        battleActions->update(inputs);
    }
    else
    {
        battleSpeech->update(inputs);
    }

    if (battleActions->isFinished())
    {
        // TODO
    }
}

void BattleScene::draw(Fps const* fps, RenderSizes rs)
{
    battleSpeech->draw(fps, rs);
    battleActions->draw(fps, rs);
}

bool BattleScene::popScene() const
{
    return false;
}

std::unique_ptr<Scene> BattleScene::nextScene()
{
    return nullptr;
}

std::string BattleScene::name()
{
    return "BattleScene";
}
