#include "bagscene.h"

BagScene::BagScene(SDL_Renderer* renderer) : Scene(renderer)
{
    for (size_t i = 0; i < NumberOfPockets; ++i)
    {
        SDL_Surface* surface = IMG_Load(("resources/Graphics/UI/Bag/bg_" + std::to_string(i + 1) + ".png").c_str());
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        backgrounds.push_back({surface, texture});
    }
    for (size_t i = 0; i < NumberOfPockets; ++i)
    {
        SDL_Surface* surface = IMG_Load(("resources/Graphics/UI/Bag/bag_" + std::to_string(i + 1) + ".png").c_str());
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        bags.push_back({surface, texture});
    }

    iconPocketSurface = IMG_Load("resources/Graphics/UI/Bag/icon_pocket.png");
    iconPocketTexture = SDL_CreateTextureFromSurface(renderer, iconPocketSurface);
}

BagScene::~BagScene()
{
    for (auto& e : backgrounds)
    {
        SDL_DestroyTexture(e.second);
        SDL_FreeSurface(e.first);
    }
    for (auto& e : bags)
    {
        SDL_DestroyTexture(e.second);
        SDL_FreeSurface(e.first);
    }

    SDL_DestroyTexture(iconPocketTexture);
    SDL_FreeSurface(iconPocketSurface);
}

void BagScene::update(Inputs const* inputs)
{
    if (inputs->B)
    {
        leave = true;
        return;
    }

    if (inputs->left)
    {
        currentPocketIndex = (currentPocketIndex + NumberOfPockets - 1) % NumberOfPockets;
        return;
    }
    else if (inputs->right)
    {
        currentPocketIndex = (currentPocketIndex + NumberOfPockets + 1) % NumberOfPockets;
        return;
    }
}

void BagScene::draw(Fps const* /*fps*/, RenderSizes rs)
{
    SDL_Surface* surface;
    SDL_Texture* texture;

    SDL_Rect srcRect;
    SDL_Rect dstRect;

    surface   = backgrounds[currentPocketIndex].first;
    texture   = backgrounds[currentPocketIndex].second;
    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.w = surface->w * rs.ww / rs.aw;
    dstRect.h = surface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    surface   = bags[currentPocketIndex].first;
    texture   = bags[currentPocketIndex].second;
    dstRect.x = (184 - surface->w) * rs.ww / rs.aw / 2;
    dstRect.y = (184 - surface->h) * rs.wh / rs.ah / 2;
    dstRect.w = surface->w * rs.ww / rs.aw;
    dstRect.h = surface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    srcRect.x = 28 * currentPocketIndex;
    srcRect.y = 0;
    srcRect.w = 28;
    srcRect.h = 28;
    dstRect.x = (2 + 22 * currentPocketIndex) * rs.ww / rs.aw;
    dstRect.y = 226 * rs.wh / rs.ah;
    dstRect.w = 28 * rs.ww / rs.aw;
    dstRect.h = 28 * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, iconPocketTexture, &srcRect, &dstRect);
}

bool BagScene::popScene() const
{
    return leave;
}

std::string BagScene::name()
{
    return "BagScene";
}
