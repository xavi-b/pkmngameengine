#include "pkmnsscene.h"

PkmnsScene::PkmnsScene(SDL_Renderer* renderer) : Scene(renderer)
{
}

PkmnsScene::~PkmnsScene()
{
}

void PkmnsScene::update(Inputs const* /*inputs*/)
{
}

void PkmnsScene::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

std::string PkmnsScene::name()
{
    return "PkmnsScene";
}
