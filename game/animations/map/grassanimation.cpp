#include "grassanimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

GrassAnimation::GrassAnimation(SDL_Renderer* renderer, bool night) : MapAnimation(renderer, night)
{
    surface = IMG_Load("resources/Graphics/Animations/Overworld dust and grass.png");
    texture = RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, surface), night);

    ticksDuration = speed * nSprites;
}

GrassAnimation::~GrassAnimation()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void GrassAnimation::start()
{
    MapAnimation::start();

    ticks = 0;
}

void GrassAnimation::incrementTicks()
{
    MapAnimation::incrementTicks();

    if (int(accumulatedTicks) % speed == 0)
        ticks++;
}

void GrassAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    static constexpr int nw = 4;

    if (ticks >= nSprites)
        return;

    int col = (spriteStartIndex + ticks) % nw;
    int row = (spriteStartIndex + ticks) / nw;

    SDL_Rect srcRect;
    srcRect.w = MapScene::TilePixelSize;
    srcRect.h = MapScene::TilePixelSize;
    srcRect.x = col * 6 * MapScene::TilePixelSize + 2.5 * MapScene::TilePixelSize;
    srcRect.y = row * 6 * MapScene::TilePixelSize + 2.5 * MapScene::TilePixelSize;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}
