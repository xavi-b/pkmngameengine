#ifndef SPRITEANIMATION_H
#define SPRITEANIMATION_H

#include "animation.h"
#include "effect.h"
#include "renderutils.h"

#include <SDL_image.h>
#include <functional>
#include <memory>
#include <vector>

class SpriteAnimation : public Animation
{
public:
    using RectLambda = std::function<SDL_Rect(SDL_Surface*, RenderSizes)>;

    SpriteAnimation(SDL_Renderer* renderer, std::string const& spritePath, int duration);
    ~SpriteAnimation();

    virtual void start() override;
    virtual void stop() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    void setRect(RectLambda const& rectLambda);
    void addEffect(std::unique_ptr<Effect>&& effect);

    SDL_Surface* surface;
    SDL_Texture* texture;

private:
    int duration = -1;

    RectLambda rectLambda;

    std::vector<std::unique_ptr<Effect>> effects;
};

#endif // SPRITEANIMATION_H
