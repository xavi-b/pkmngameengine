#include "keyboard.h"

#include "settings.h"
#include "game.h"

Keyboard::Keyboard(SDL_Renderer* renderer, std::string& playerName) : Scene(renderer), playerName(playerName)
{
}

Keyboard::~Keyboard()
{
}

void Keyboard::init()
{
}

void Keyboard::update(const Inputs* inputs)
{
    if (inputs->A)
    {
        playerName = "Red";
        leave      = true;
    }
}

void Keyboard::draw(const Fps* /*fps*/, RenderSizes /*rs*/)
{
}

bool Keyboard::popScene() const
{
    return leave;
}

std::unique_ptr<Scene> Keyboard::nextScene()
{
    return nullptr;
}

std::string Keyboard::name()
{
    return "Keyboard";
}
