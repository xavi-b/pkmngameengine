#ifndef LEDGEANIMATION_H
#define LEDGEANIMATION_H

#include "entity.h"
#include "mapanimation.h"
#include "sprite.h"

class LedgeAnimation : public MapAnimation
{
public:
    LedgeAnimation(SDL_Renderer* renderer, Entity::Direction direction, bool night);
    ~LedgeAnimation();

    virtual bool isFinished() const override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    void setEntitySprite(Entity* entity, Sprite* sprite);

    Entity::Direction getDirection() const;

    void                setStartingPosition(int x, int y);
    std::pair<int, int> getStartingPosition() const;
    std::pair<int, int> getFinishPosition() const;

private:
    Entity* entity = nullptr;
    Sprite* sprite = nullptr;

    SDL_Surface* surface;
    SDL_Texture* texture;

    std::pair<int, int> startingPosition;

    Entity::Direction direction = Entity::Direction::NONE;
};

#endif // LEDGEANIMATION_H
