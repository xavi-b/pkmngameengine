#include "leavescene.h"

#include "game.h"
#include "utils.h"

LeaveScene::LeaveScene(SDL_Renderer* renderer) : Scene(renderer)
{
    quitSpeech = std::make_unique<TextSpeech>(renderer);
    std::vector<std::string> texts;
    texts.push_back(lc::translate("Quit ?"));
    quitSpeech->setTexts(texts);
    quitSpeech->start();
    quitQuestion = std::make_unique<TextQuestion>(renderer);
    quitQuestion->setTexts({lc::translate("Yes"), lc::translate("No")});
    quitQuestion->init();
}

LeaveScene::~LeaveScene()
{
}

void LeaveScene::init()
{
}

void LeaveScene::update(Inputs const* inputs)
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

void LeaveScene::draw(Fps const* fps, RenderSizes rs)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    quitSpeech->draw(fps, rs);
    quitQuestion->draw(fps, rs, TextQuestion::RIGHT);
}

bool LeaveScene::popScene() const
{
    return quitQuestion->isFinished();
}

std::unique_ptr<Scene> LeaveScene::nextScene()
{
    return nullptr;
}

std::string LeaveScene::name()
{
    return "LeaveScene";
}
