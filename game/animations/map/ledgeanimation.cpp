#include "ledgeanimation.h"

#include "scenes/mapscene.h"

#include <SDL_image.h>

LedgeAnimation::LedgeAnimation(SDL_Renderer* renderer, Entity::Direction direction, bool night)
    : MapAnimation(renderer, night), direction(direction)
{
    surface = IMG_Load("resources/Graphics/Characters/shadow.png");
    texture = RenderUtils::texture(renderer, SDL_CreateTextureFromSurface(renderer, surface), night);
}

LedgeAnimation::~LedgeAnimation()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

bool LedgeAnimation::isFinished() const
{
    if (!entity)
        return false;

    switch (direction)
    {
    case Entity::Direction::LEFT:
        return entity->previousX == startingPosition.first - 2;
    case Entity::Direction::RIGHT:
        return entity->previousX == startingPosition.first + 2;
    case Entity::Direction::UP:
        return entity->previousY == startingPosition.second - 2;
    case Entity::Direction::DOWN:
        return entity->previousY == startingPosition.second + 2;
    default:
        return true;
    }
}

void LedgeAnimation::draw(Fps const* fps, RenderSizes rs)
{
    float speed = (entity->speed + entity->previousSpeed) / 2.0;

    if (accumulatedTicks > speed / 2)
    {
        SDL_Rect shadowRect = dstRect;
        shadowRect.h /= 3;
        shadowRect.y += (MapScene::EntityPixelHeight - MapScene::TilePixelSize) * rs.wh / rs.ah + shadowRect.h;
        SDL_RenderCopy(renderer, texture, NULL, &shadowRect);

        int dstTilePixelHeight = MapScene::TilePixelSize * rs.wh / rs.ah;

        int delta = entity->y - startingPosition.second;
        if (direction == Entity::Direction::LEFT || direction == Entity::Direction::RIGHT)
            delta = entity->x - startingPosition.first;

        float minusonetoplusone = (sprite->getAccumulatedTicks() + fps->tickPercentage() - 0.5) / (speed * 2);
        float sinus             = std::cos(std::numbers::pi / 2 * delta * minusonetoplusone);

        int result = dstTilePixelHeight * sinus / 2;

        switch (direction)
        {
        case Entity::Direction::LEFT:
        case Entity::Direction::RIGHT:
            dstRect.y -= result;
            break;
        case Entity::Direction::DOWN:
            dstRect.y -= result / 2;
            break;
        default:
            break;
        }
    }

    sprite->updateSpriteRow(*entity);
    sprite->draw(*entity, fps, rs, dstRect);
}

void LedgeAnimation::setEntitySprite(Entity* entity, Sprite* sprite)
{
    this->entity = entity;
    this->sprite = sprite;
}

Entity::Direction LedgeAnimation::getDirection() const
{
    return direction;
}

void LedgeAnimation::setStartingPosition(int x, int y)
{
    startingPosition = {x, y};
}

std::pair<int, int> LedgeAnimation::getStartingPosition() const
{
    return startingPosition;
}

std::pair<int, int> LedgeAnimation::getFinishPosition() const
{
    switch (direction)
    {
    case Entity::Direction::LEFT:
        return {startingPosition.first - 2, startingPosition.second};
    case Entity::Direction::RIGHT:
        return {startingPosition.first + 2, startingPosition.second};
    case Entity::Direction::UP:
        return {startingPosition.first, startingPosition.second - 2};
    case Entity::Direction::DOWN:
        return {startingPosition.first, startingPosition.second + 2};
    default:
        return {};
    }
}
