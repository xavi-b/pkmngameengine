#include "battle.h"

Battle::Battle(SDL_Renderer* renderer)
    : Scene(renderer)
{
}

void Battle::update(const Inputs* inputs)
{
    if (inputs->B)
        goToNextScene = true;
}

void Battle::draw(const Fps* fps, int w, int h)
{
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
}

#include "scenes/titlescene.h"
std::unique_ptr<Scene> Battle::nextScene() const
{
    if (goToNextScene)
        return std::make_unique<TitleScene>(renderer);
    return nullptr;
}

std::string Battle::name()
{
    return "Battle";
}
