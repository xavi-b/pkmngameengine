#include "sprite.h"

#include "fps.h"
#include "scenes/mapscene.h"

#include <SDL_image.h>

Sprite::Sprite(SDL_Renderer* renderer) : renderer(renderer)
{
}

Sprite::~Sprite()
{
    if (texture)
        SDL_DestroyTexture(texture);
    if (surface)
        SDL_FreeSurface(surface);
}

bool Sprite::load(std::string const& spritePath, bool night)
{
    if (texture)
        SDL_DestroyTexture(texture);
    if (surface)
        SDL_FreeSurface(surface);

    surface = IMG_Load(spritePath.c_str());
    if (!surface)
        return false;

    texture = RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, surface), night);
    if (!texture)
        return false;

    return true;
}

void Sprite::forceSpriteDirection(Entity::Direction direction)
{
    switch (direction)
    {
    case Entity::Direction::UP:
        spriteRow = 3;
        break;
    case Entity::Direction::DOWN:
        spriteRow = 0;
        break;
    case Entity::Direction::LEFT:
        spriteRow = 1;
        break;
    case Entity::Direction::RIGHT:
        spriteRow = 2;
        break;
    default:
        spriteRow = 0;
        break;
    }
}

void Sprite::draw(Entity const& entity, Fps const* fps, SDL_Rect* dstRect)
{
    switch (entity.direction)
    {
    case Entity::Direction::UP:
        spriteRow = 3;
        break;
    case Entity::Direction::DOWN:
        spriteRow = 0;
        break;
    case Entity::Direction::LEFT:
        spriteRow = 1;
        break;
    case Entity::Direction::RIGHT:
        spriteRow = 2;
        break;
    default:
        break;
    }

    int imageCol = 0;
    if (entity.direction != Entity::Direction::NONE)
    {
        imageCol = std::floor((accumulatedTicks + fps->tickPercentage()) / entity.speed * 4);
    }

    SDL_Rect srcRect;
    srcRect.x = imageCol * MapScene::TilePixelSize;
    srcRect.y = spriteRow * MapScene::EntityPixelHeight;
    srcRect.w = MapScene::TilePixelSize;
    srcRect.h = MapScene::EntityPixelHeight;

    SDL_RenderCopy(renderer, texture, &srcRect, dstRect);
}

size_t Sprite::getAccumulatedTicks() const
{
    return accumulatedTicks;
}

void Sprite::setAccumulatedTicks(size_t newAccumulatedTicks)
{
    accumulatedTicks = newAccumulatedTicks;
}
