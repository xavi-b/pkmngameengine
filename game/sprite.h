#ifndef SPRITE_H
#define SPRITE_H

#include "entity.h"
#include "fps.h"

#include <SDL_render.h>
#include <string>

class Sprite
{
public:
    Sprite(SDL_Renderer* renderer);
    ~Sprite();

    bool load(std::string const& spritePath, bool night);

    void forceSpriteDirection(Entity::Direction direction);

    virtual void updateSpriteRow(Entity const& entity);
    virtual void draw(Entity const& entity, Fps const* fps, RenderSizes rs, SDL_Rect dstRect);

    size_t getAccumulatedTicks() const;
    void   setAccumulatedTicks(size_t newAccumulatedTicks);

protected:
    SDL_Renderer* renderer;
    SDL_Surface*  surface          = NULL;
    SDL_Texture*  texture          = NULL;
    size_t        spriteRow        = 0; // So we draw DOWN as default
    size_t        accumulatedTicks = 0;
};

#endif // SPRITE_H
