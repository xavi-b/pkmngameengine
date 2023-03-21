#include "introscene.h"

#include "keyboardscene.h"
#include "game.h"
#include "utils.h"

#include <iostream>

IntroScene::IntroScene(SDL_Renderer* renderer) : Scene(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Pictures/introbg.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);

    std::vector<std::string> texts;
    texts.push_back(lc::translate("Prof: Hi !"));
    texts.push_back(
        lc::translate("Please give me your name, young person ! I need it for you to continue your adventure !"));
    introSpeech = std::make_unique<TextSpeech>(renderer);
    introSpeech->setTexts(texts);
    genderSpeech = std::make_unique<TextSpeech>(renderer);
}

IntroScene::~IntroScene()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
}

void IntroScene::init()
{
    introSpeech->init();
}

void IntroScene::update(const Inputs* inputs)
{
    switch (state)
    {
    case IntroSpeech:
        introSpeech->update(inputs);
        if (introSpeech->isFinished())
        {
            Game::instance()->data.player.name.clear();
            state = Keyboard;
        }
        break;
    case Keyboard:
        std::cout << "PlayerName: " << Game::instance()->data.player.name << std::endl;
        if (!Game::instance()->data.player.name.empty())
        {
            std::vector<std::string> texts;
            texts.push_back(lc::translate("Prof: So your name is ").str() + Game::instance()->data.player.name);
            texts.push_back(lc::translate("Are you a boy or a girl ?"));
            genderSpeech->setTexts(texts);
            genderSpeech->init();
            genderSpeech->update(inputs);
            state = GenderSpeech;
        }
        break;
    case GenderSpeech:
        genderSpeech->update(inputs);
        break;
    case Leave:
        // TODO save game
        break;
    default:
        break;
    }
}

void IntroScene::draw(const Fps* fps, RenderSizes rs)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    switch (state)
    {
    case IntroSpeech:
        introSpeech->draw(fps, rs);
        break;
    case Keyboard:
        break;
    case GenderSpeech:
        genderSpeech->draw(fps, rs);
        break;
    case Leave:
        break;
    default:
        break;
    }
}

bool IntroScene::pushScene() const
{
    return state == Keyboard;
}

std::unique_ptr<Scene> IntroScene::nextScene()
{
    if (state == Keyboard)
    {
        introSpeech->reset();
        return std::make_unique<KeyboardScene>(renderer, Game::instance()->data.player.name);
    }
    return nullptr;
}

std::string IntroScene::name()
{
    return "IntroScene";
}

void IntroScene::debug()
{
    std::cout << name() << " state: " << state << std::endl;
    std::cout << name() << " gender finished: " << genderSpeech->isFinished() << std::endl;
}
