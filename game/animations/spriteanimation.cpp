#include "spriteanimation.h"

SpriteFrame::SpriteFrame(SDL_Renderer* renderer, const std::string& spritePath, SDL_Rect dstRect, int duration)
    : renderer(renderer), dstRect(dstRect), duration(duration)
{
    surface = IMG_Load(spritePath.c_str());
    texture = SDL_CreateTextureFromSurface(renderer, surface);
}

SpriteFrame::~SpriteFrame()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void SpriteFrame::draw(const Fps* /*fps*/, RenderSizes /*rs*/)
{
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}

void SpriteFrame::reset()
{
    ticks = 0;
}

void SpriteFrame::incrementTicks()
{
    ticks++;
}

bool SpriteFrame::isFinished() const
{
    return duration >= 0 && ticks >= duration;
}

SpriteAnimation::SpriteAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
}

void SpriteAnimation::reset()
{
    currentFrame = 0;
    if (currentFrame < frames.size())
        frames[currentFrame]->reset();
    Animation::reset();
}

void SpriteAnimation::incrementTicks()
{
    if (currentFrame < frames.size())
    {
        frames[currentFrame]->incrementTicks();
        if (frames[currentFrame]->isFinished())
        {
            // frames[currentFrame]->reset();
            currentFrame++;
            if (currentFrame >= frames.size())
            {
                stop();
            }
        }
    }
}

void SpriteAnimation::draw(const Fps* fps, RenderSizes rs)
{
    if (currentFrame < frames.size())
        frames[currentFrame]->draw(fps, rs);
}

void SpriteAnimation::forceEnd()
{
    if (frames.size() > 0)
        currentFrame = frames.size() - 1;
    else
        currentFrame = 0;
    running = false;
}
