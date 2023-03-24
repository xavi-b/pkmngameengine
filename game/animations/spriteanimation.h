#ifndef SPRITEANIMATION_H
#define SPRITEANIMATION_H

#include <vector>
#include <memory>
#include <functional>
#include "animation.h"
#include "renderutils.h"
#include "effect.h"
#include <SDL_image.h>

class SpriteAnimation : public Animation
{
public:
    using RectLambda = std::function<SDL_Rect(SDL_Surface*, RenderSizes)>;

    SpriteAnimation(SDL_Renderer* renderer, const std::string& spritePath, int duration);
    ~SpriteAnimation();

    virtual void start() override;
    virtual void stop() override;
    virtual void incrementTicks() override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;

    void setRect(const RectLambda& rectLambda);
    void addEffect(std::unique_ptr<Effect>&& effect);

    SDL_Surface* surface;
    SDL_Texture* texture;

private:
    int duration = -1;

    RectLambda rectLambda;

    std::vector<std::unique_ptr<Effect>> effects;
};

#endif // SPRITEANIMATION_H
