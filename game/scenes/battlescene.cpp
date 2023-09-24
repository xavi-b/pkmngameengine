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
    quitQuestion = std::make_unique<TextQuestion>(renderer);
    quitQuestion->setTexts({lc::translate("Yes"), lc::translate("No")});
    quitQuestion->init();
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
        quitQuestion->update(inputs);
    }
    else
    {
        quitSpeech->update(inputs);
    }

    if (quitQuestion->isFinished())
    {
        if (quitQuestion->selectedIndex() == 0)
        {
            Game::instance()->quit();
        }
    }
}

void BattleScene::draw(Fps const* fps, RenderSizes rs)
{
    quitSpeech->draw(fps, rs);
    quitQuestion->draw(fps, rs, TextQuestion::Right);
}

bool BattleScene::popScene() const
{
    return quitQuestion->isFinished();
}

std::unique_ptr<Scene> BattleScene::nextScene()
{
    return nullptr;
}

std::string BattleScene::name()
{
    return "BattleScene";
}
