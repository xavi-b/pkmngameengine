#ifndef STAIRSANIMATION_H
#define STAIRSANIMATION_H

#include "entity.h"
#include "mapanimation.h"
#include "sprite.h"

class StairsAnimation : public MapAnimation
{
public:
    enum Direction
    {
        ToUpstairs,
        ToDownstairs,
        FromUpstairs,
        FromDownstairs
    };

    StairsAnimation(SDL_Renderer* renderer, Direction direction, bool night);

    virtual bool isFinished() const override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    void setEntitySprite(Entity* entity, Sprite* sprite);
    void setInverted(bool newInverted);

private:
    Entity* entity = nullptr;
    Sprite* sprite = nullptr;

    bool inverted = false;

    Direction direction = ToUpstairs;
};

#endif // STAIRSANIMATION_H
