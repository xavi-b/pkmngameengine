#include "sandanimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

SandAnimation::SandAnimation(SDL_Renderer* renderer, bool night)
    : TileAnimation(renderer,
                    "resources/Graphics/Animations/Overworld sand.png",
                    TileLayer::Type::GROUND_OVERLAY,
                    night)
{
    speed    = 2;
    nSprites = 4;
}

void SandAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    // TODO bikes

    if (ticks >= nSprites)
        return;

    int col = 0;

    switch (direction)
    {
    case Entity::Direction::LEFT:
        spriteStartIndex = 0;
        break;
    case Entity::Direction::RIGHT:
        spriteStartIndex = 1;
        break;
    case Entity::Direction::UP:
        spriteStartIndex = 2;
        break;
    case Entity::Direction::DOWN:
        spriteStartIndex = 3;
        break;
    case Entity::Direction::NONE:
        break;
    }

    col = spriteStartIndex;

    int row = 2 * ticks;

    SDL_Rect srcRect;
    srcRect.w = MapScene::TilePixelSize;
    srcRect.h = MapScene::TilePixelSize;
    srcRect.x = col * 6 * MapScene::TilePixelSize + 2.5 * MapScene::TilePixelSize;
    srcRect.y = row * 6 * MapScene::TilePixelSize + 2.5 * MapScene::TilePixelSize;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}
