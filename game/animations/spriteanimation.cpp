#include "spriteanimation.h"

SpriteAnimation::SpriteAnimation(SDL_Renderer* renderer, const std::string& spritePath, int duration)
    : Animation(renderer), duration(duration)
{
    surface = IMG_Load(spritePath.c_str());
    texture = SDL_CreateTextureFromSurface(renderer, surface);
}

SpriteAnimation::~SpriteAnimation()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void SpriteAnimation::draw(const Fps* /*fps*/, RenderSizes rs)
{
    SDL_Rect dstRect;

    int dstWidth  = surface->w * rs.ww / rs.aw;
    int dstHeight = surface->h * rs.wh / rs.ah;

    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.w = dstWidth;
    dstRect.h = dstHeight;

    for (auto& e : effects)
        e->apply(ticks, duration, dstRect);

    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}

void SpriteAnimation::reset()
{
    ticks = 0;
}

void SpriteAnimation::incrementTicks()
{
    ticks++;
}

bool SpriteAnimation::isFinished() const
{
    return duration >= 0 && ticks >= duration;
}

void SpriteAnimation::addEffect(std::unique_ptr<Effect>&& effect)
{
    this->effects.emplace_back(std::move(effect));
}
