#include "battlescene.h"

#include "game.h"
#include "utils.h"

BattleScene::BattleScene(SDL_Renderer* renderer) : Scene(renderer)
{
    quitSpeech = std::make_unique<TextSpeech>(renderer);
    std::vector<std::string> texts;
    texts.push_back(lc::translate("Quit ?"));
    quitSpeech->setTexts(texts);
    quitSpeech->init();
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
    if (quitSpeech->mayClose())
    {
        battleActions->update(inputs);
    }
    else
    {
        quitSpeech->update(inputs);
    }

    if (battleActions->isFinished())
    {
        // TODO
    }
}

void BattleScene::draw(Fps const* fps, RenderSizes rs)
{
    quitSpeech->draw(fps, rs);
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
