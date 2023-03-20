#include "introscene.h"

#include "keyboard.h"
#include "game.h"

IntroScene::IntroScene(SDL_Renderer* renderer) : Scene(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Pictures/introbg.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);

    std::vector<std::string> texts;
    texts.push_back("Prof: Hi !");
    texts.push_back("Please give me your name, young person ! I need it for you to continue your adventure !");
    speech = std::make_unique<TextSpeech>(renderer, texts);
}

IntroScene::~IntroScene()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
}

void IntroScene::init()
{
}

void IntroScene::update(const Inputs* inputs)
{
    // TODO
    speech->update(inputs);
    if (speech->isFinished())
        showKeyboardScene = true;
}

void IntroScene::draw(const Fps* fps, RenderSizes rs)
{
    speech->draw(fps, rs);
}

bool IntroScene::pushScene() const
{
    return true;
}

std::unique_ptr<Scene> IntroScene::nextScene()
{
    if (showKeyboardScene)
    {
        showKeyboardScene = false;
        return std::make_unique<Keyboard>(renderer, Game::instance()->data.player.name);
    }
    return nullptr;
}

std::string IntroScene::name()
{
    return "IntroScene";
}
