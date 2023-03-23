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

void SpriteAnimation::draw(const Fps* /*fps*/, RenderSizes /*rs*/)
{
    float pos = 1.0 * ticks / duration;

    /*
    SDL_Rect dstRect;
    dstRect.x = startRect.x + (endRect.x - startRect.x) * pos;
    dstRect.y = startRect.y + (endRect.y - startRect.y) * pos;
    dstRect.w = startRect.w + (endRect.w - startRect.w) * pos;
    dstRect.h = startRect.h + (endRect.h - startRect.h) * pos;
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    */

    // TODO
    // SDL_SetTextureColorMod
    // Effects
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

void SpriteAnimation::setEffects(std::vector<std::unique_ptr<Effect>>&& effects)
{
    this->effects = std::move(effects);
}
