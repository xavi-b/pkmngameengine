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

    bool load(std::string const& spritePath);

    void forceSpriteDirection(Entity::Direction direction);

    virtual void draw(Entity const& entity, Fps const* fps, size_t accumulatedTicks, SDL_Rect* dstRect);

private:
    SDL_Renderer* renderer;
    SDL_Surface*  surface   = NULL;
    SDL_Texture*  texture   = NULL;
    size_t        spriteRow = 0; // So we draw DOWN as default
};

#endif // SPRITE_H
