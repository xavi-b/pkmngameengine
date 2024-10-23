#include "introscene.h"

#include "game.h"
#include "keyboardscene.h"
#include "scenes/story/town1scene.h"
#include "utils.h"

#include <iostream>

IntroScene::IntroScene(SDL_Renderer* renderer) : Scene(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Pictures/introbg.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);

    introSpeech = std::make_unique<TextSpeech>(renderer);
    introSpeech->setTexts(
        {lc::translate("Prof: Hi !"),
         lc::translate("Please give me your name, young person ! I need it for you to continue your adventure !")});
    genderSpeech   = std::make_unique<TextSpeech>(renderer);
    genderQuestion = std::make_unique<TextQuestion>(renderer);
    genderQuestion->setTexts({lc::translate("Boy"), lc::translate("Girl")});
    outroSpeech = std::make_unique<TextSpeech>(renderer);
    outroSpeech->setTexts({lc::translate("Welcome then !")});

    introAnimation = std::make_unique<IntroAnimation>(renderer);
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

void IntroScene::update(Inputs const* inputs)
{
    introAnimation->incrementTicks();

    switch (state)
    {
    case IntroSpeech:
        if (!introAnimation->isStarted())
            introAnimation->start();
        introSpeech->update(inputs);
        if (introSpeech->shouldClose())
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
        if (genderSpeech->mayClose())
        {
            genderQuestion->init();
            state = GenderQuestion;
        }
        else
        {
            genderSpeech->update(inputs);
        }
        break;
    case GenderQuestion: {
        genderQuestion->update(inputs);
        Player::Gender gender = static_cast<Player::Gender>(genderQuestion->selectedIndex());
        if (genderQuestion->isFinished())
        {
            Game::instance()->data.player.gender = gender;
            std::vector<std::string> texts;
            texts.push_back(
                lc::translate("Prof: So you are a ").str()
                + (Game::instance()->data.player.gender == 0 ? lc::translate("boy") : lc::translate("girl")).str()
                + ".");
            outroSpeech->setTexts(texts);
            outroSpeech->init();
            outroSpeech->update(inputs);
            // TODO temp
            PkmnDef::PkmnDefPtr definition = Game::instance()->data.pkmns.front();
            auto                pkmn       = std::make_shared<Pkmn>(definition, 5);
            Game::instance()->data.player.pkmns.at(0).swap(pkmn);
            state = OutroSpeech;
        }
        break;
    }
    case OutroSpeech:
        outroSpeech->update(inputs);
        if (outroSpeech->shouldClose())
        {
            state = Leave;
        }
        break;
    case Leave:
        Game::instance()->save();
        break;
    }
}

void IntroScene::draw(Fps const* fps, RenderSizes rs)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    introAnimation->draw(fps, rs);

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
    case GenderQuestion:
        genderSpeech->draw(fps, rs);
        genderQuestion->draw(fps, rs, TextQuestion::RIGHT);
        break;
    case OutroSpeech:
        outroSpeech->draw(fps, rs);
        break;
    case Leave:
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
    else if (state == Leave)
    {
        auto scene = std::make_unique<Town1Scene>(renderer);
        scene->initPlayerPosition(8, 12, MapScene::NONE);
        return scene;
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
    std::cout << name() << " gender shouldClose: " << genderSpeech->shouldClose() << std::endl;
    std::cout << name() << " gender mayClose: " << genderSpeech->mayClose() << std::endl;
}
