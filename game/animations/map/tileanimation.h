#ifndef TILEANIMATION_H
#define TILEANIMATION_H

#include "layers/tilelayer.h"
#include "mapanimation.h"

class TileAnimation : public MapAnimation
{
public:
    TileAnimation(SDL_Renderer* renderer, std::string const& tilePath, TileLayer::Type layerType, bool night);
    ~TileAnimation();

    virtual void start() override;
    virtual void incrementTicks() override;

    Entity::Direction getDirection() const;
    void              setDirection(Entity::Direction direction);

    TileLayer::Type getLayerType() const;

protected:
    SDL_Surface* surface;
    SDL_Texture* texture;

    TileLayer::Type   layerType;
    Entity::Direction direction;

    int speed            = 1;
    int spriteStartIndex = 0;
    int nSprites         = 8;
    int ticks            = 0;
};

#endif // TILEANIMATION_H
