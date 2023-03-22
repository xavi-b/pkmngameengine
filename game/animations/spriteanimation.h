#ifndef SPRITEANIMATION_H
#define SPRITEANIMATION_H

#include <vector>
#include <memory>
#include "animation.h"
#include "renderutils.h"
#include <SDL_image.h>

class SpriteFrame
{
public:
    SpriteFrame(SDL_Renderer* renderer, const std::string& spritePath, SDL_Rect dstRect, int duration);
    ~SpriteFrame();

    void draw(const Fps* fps, RenderSizes rs);
    void reset();
    void incrementTicks();
    bool isFinished() const;

private:
    SDL_Renderer* renderer;

    SDL_Surface* surface;
    SDL_Texture* texture;

    int ticks = 0;

    SDL_Rect dstRect;
    int      duration = -1;
};

class SpriteAnimation : public Animation
{
public:
    SpriteAnimation(SDL_Renderer* renderer);

    virtual void reset() override;
    virtual void incrementTicks() override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;
    virtual void forceEnd() override;

protected:
    unsigned int currentFrame = 0;

    std::vector<std::unique_ptr<SpriteFrame>> frames;
};

#endif // SPRITEANIMATION_H
