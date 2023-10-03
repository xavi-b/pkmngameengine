#include "bagscene.h"

BagScene::BagScene(SDL_Renderer* renderer) : Scene(renderer)
{
}

BagScene::~BagScene()
{
}

void BagScene::update(Inputs const* /*inputs*/)
{
}

void BagScene::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

std::string BagScene::name()
{
    return "BagScene";
}
