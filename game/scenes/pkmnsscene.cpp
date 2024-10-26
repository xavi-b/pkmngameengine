#include "pkmnsscene.h"

PkmnsScene::PkmnsScene(SDL_Renderer* renderer, Pkmn::PkmnPtr& currentPkmn) : Scene(renderer), selectedPkmn(currentPkmn)
{
}

PkmnsScene::~PkmnsScene()
{
}

void PkmnsScene::update(Inputs const* inputs)
{
    if (inputs->B)
    {
        selectedPkmn = nullptr;
        leave        = true;
    }
}

void PkmnsScene::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

bool PkmnsScene::popScene() const
{
    return leave;
}

std::string PkmnsScene::name()
{
    return "PkmnsScene";
}

void PkmnsScene::selectPkmn(Pkmn::PkmnPtr newSelectedPkmn)
{
    selectedPkmn = newSelectedPkmn;
    leave        = true;
}
