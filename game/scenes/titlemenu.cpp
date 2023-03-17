#include "titlemenu.h"

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
    //    if (inputs->B)
    //        goToNextScene = true;
}

void TitleMenu::draw(const Fps* fps)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
}

std::unique_ptr<Scene> TitleMenu::nextScene() const
{
    return nullptr;
}
