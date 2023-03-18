#include "titlemenu.h"

#include "titlescene.h"

TitleMenu::TitleMenu(SDL_Renderer* renderer)
    : Scene(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Pictures/loadbg.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
}

TitleMenu::~TitleMenu()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
}

void TitleMenu::sendInputs(const Inputs* inputs)
{
    this->inputs = *inputs;
}

void TitleMenu::draw(const Fps* fps)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
}

std::unique_ptr<Scene> TitleMenu::nextScene() const
{
    if (inputs.B)
        return std::make_unique<TitleScene>(renderer);
    return nullptr;
}
