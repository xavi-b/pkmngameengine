#include "tileanimation.h"

#include <SDL_image.h>

TileAnimation::TileAnimation(SDL_Renderer* renderer, std::string const& tilePath, TileLayer::Type layerType, bool night)
    : MapAnimation(renderer, night), layerType(layerType)
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

Entity::Direction TileAnimation::getDirection() const
{
    return direction;
}

void TileAnimation::setDirection(Entity::Direction direction)
{
    this->direction = direction;
}

TileLayer::Type TileAnimation::getLayerType() const
{
    return layerType;
}
