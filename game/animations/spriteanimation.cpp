#include "spriteanimation.h"

#include <iostream>

SpriteAnimation::SpriteAnimation(SDL_Renderer* renderer, std::string const& spritePath, int duration)
    : Animation(renderer), duration(duration)
{
    surface = IMG_Load(spritePath.c_str());
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    rectLambda = [](SDL_Surface* surface, RenderSizes rs) {
        SDL_Rect dstRect;

        int dstWidth  = surface->w * rs.ww / rs.aw;
        int dstHeight = surface->h * rs.wh / rs.ah;

        dstRect.x = 0;
        dstRect.y = 0;
        dstRect.w = dstWidth;
        dstRect.h = dstHeight;

        return dstRect;
    };
}

SpriteAnimation::~SpriteAnimation()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void SpriteAnimation::start()
{
    Animation::start();
}

void SpriteAnimation::stop()
{
    Animation::stop();
}

void SpriteAnimation::incrementTicks()
{
    if (!isStarted() || isFinished())
        return;

    accumulatedTicks++;
    if (accumulatedTicks >= duration)
        forceEnd();
}

void SpriteAnimation::draw(Fps const* fps, RenderSizes rs)
{
    SDL_Rect dstRect = rectLambda(surface, rs);

    float t = 0;
    if (isStarted())
        t = (isFinished() ? duration : (accumulatedTicks + fps->tickPercentage()));

    for (auto& e : effects)
        e->apply(t, duration, rs, dstRect);

    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}

void SpriteAnimation::setRect(RectLambda const& rectLambda)
{
    this->rectLambda = rectLambda;
}

void SpriteAnimation::addEffect(std::unique_ptr<Effect>&& effect)
{
    this->effects.emplace_back(std::move(effect));
}
