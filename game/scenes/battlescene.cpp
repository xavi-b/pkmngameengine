#include "battlescene.h"

#include "game.h"
#include "utils.h"

BattleScene::BattleScene(SDL_Renderer* renderer) : Scene(renderer)
{
    battleBackground = std::make_unique<BattleBackground>(renderer);
    battleSpeech     = std::make_unique<BattleSpeech>(renderer);
    std::vector<std::string> texts;
    texts.push_back(lc::translate("What should AAAAAAAAAAAA do ?"));
    battleSpeech->setTexts(texts);
    battleSpeech->init();
    battleActions = std::make_unique<BattleActions>(renderer);
    battleActions->init();
    moveSelection = std::make_unique<MoveSelection>(renderer);
    moveSelection->init();
}

BattleScene::~BattleScene()
{
}

void BattleScene::init()
{
    state = MENU;
}

void BattleScene::update(Inputs const* inputs)
{
    if (state == MENU)
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
            if (battleActions->selectedAction() == BattleActions::MOVES)
            {
                state = MOVES;
            }
            else if (battleActions->selectedAction() == BattleActions::RUN)
            {
                state = RUN;
            }
            battleActions->reset();
        }
    }
    else if (state == MOVES)
    {
        moveSelection->update(inputs);
        if (moveSelection->shouldQuit())
        {
            state = MENU;
        }
        else if (moveSelection->selectedIndex())
        {
            // TODO
        }
    }
}

void BattleScene::draw(Fps const* fps, RenderSizes rs)
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

bool BattleScene::popScene() const
{
    return state == RUN;
}

std::unique_ptr<Scene> BattleScene::nextScene()
{
    return nullptr;
}

std::string BattleScene::name()
{
    return "BattleScene";
}
