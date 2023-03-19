#include "introscene.h"

#include "keyboard.h"
#include "game.h"

IntroScene::IntroScene(SDL_Renderer* renderer)
    : Scene(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Pictures/introbg.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
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
    if (inputs->A)
        showKeyboardScene = true;
}

void IntroScene::draw(const Fps* /*fps*/, RenderSizes /*rs*/)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
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
