#ifndef SPRITEANIMATION_H
#define SPRITEANIMATION_H

#include <vector>
#include <memory>
#include "animation.h"
#include "renderutils.h"
#include "effect.h"
#include <SDL_image.h>

class SpriteAnimation : public Animation
{
public:
    SpriteAnimation(SDL_Renderer* renderer, const std::string& spritePath, int duration);
    ~SpriteAnimation();

    virtual void draw(const Fps* fps, RenderSizes rs) override;
    virtual void reset() override;
    virtual void incrementTicks() override;
    virtual bool isFinished() const override;

    void setEffects(std::vector<std::unique_ptr<Effect>>&& effects);

private:
    SDL_Renderer* renderer;

    SDL_Surface* surface;
    SDL_Texture* texture;

    int ticks    = 0;
    int duration = -1;

    std::vector<std::unique_ptr<Effect>> effects;
};

#endif // SPRITEANIMATION_H
