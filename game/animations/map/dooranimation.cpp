#include "dooranimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

DoorAnimation::DoorAnimation(SDL_Renderer* renderer) : MapAnimation(renderer)
{
}

DoorAnimation::~DoorAnimation()
{
}

void DoorAnimation::start()
{
    MapAnimation::start();

    ticks = speed;
}

void DoorAnimation::incrementTicks()
{
    MapAnimation::incrementTicks();

    if (int(accumulatedTicks) % speed == 0)
        ticks++;
}

bool DoorAnimation::isFinished() const
{
    if (ticks > 0 && nSprites != 0 && ticks % nSprites == nSprites - 1)
        return true;

    return MapAnimation::isFinished();
}

void DoorAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    SDL_Rect srcRect = this->srcRect;
    if (!inverted)
        srcRect.y = (ticks % nSprites) * MapScene::TilePixelSize;
    else
        srcRect.y = surface->h - (ticks % nSprites) * MapScene::TilePixelSize - MapScene::TilePixelSize;
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}

void DoorAnimation::setSprite(std::pair<SDL_Surface*, SDL_Texture*> sprite)
{
    surface = sprite.first;
    texture = sprite.second;

    nSprites      = surface->h / MapScene::TilePixelSize;
    ticksDuration = speed * nSprites;
}

void DoorAnimation::setSourceRect(SDL_Rect const& srcRect)
{
    this->srcRect = srcRect;
}

void DoorAnimation::setInverted(bool newInverted)
{
    inverted = newInverted;
}
