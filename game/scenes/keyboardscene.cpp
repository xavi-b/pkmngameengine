#include "keyboardscene.h"

KeyboardScene::KeyboardScene(SDL_Renderer* renderer, std::string& textOut, size_t maxSize) : Scene(renderer)
{
    keyboard = std::make_unique<Keyboard>(renderer, textOut, maxSize);
}

KeyboardScene::~KeyboardScene()
{
}

void KeyboardScene::init()
{
    keyboard->init();
}

void KeyboardScene::update(Inputs const* inputs)
{
    keyboard->update(inputs);
}

void KeyboardScene::draw(Fps const* fps, RenderSizes rs)
{
    keyboard->draw(fps, rs);
}

bool KeyboardScene::popScene() const
{
    return keyboard->isFinished();
}

std::unique_ptr<Scene> KeyboardScene::nextScene()
{
    return nullptr;
}

std::string KeyboardScene::name()
{
    return "KeyboardScene";
}
