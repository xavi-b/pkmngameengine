#include "tileanimation.h"

#include <SDL_image.h>

TileAnimation::TileAnimation(SDL_Renderer* renderer, std::string const& tilePath, bool night)
    : MapAnimation(renderer, night)
{
    surface = IMG_Load(tilePath.c_str());
    texture = RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, surface), night);

    ticksDuration = speed * nSprites;
}

TileAnimation::~TileAnimation()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void TileAnimation::start()
{
    MapAnimation::start();

    ticks = 0;
}

void TileAnimation::incrementTicks()
{
    MapAnimation::incrementTicks();

    if (int(accumulatedTicks) % speed == 0)
        ticks++;
}
