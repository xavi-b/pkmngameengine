#include "introanimation.h"

#include "parallelanimation.h"
#include "sequentialanimation.h"
#include "spriteanimation.h"

IntroAnimation::IntroAnimation(SDL_Renderer* renderer) : SequentialAnimation(renderer)
{
    int duration;

    auto parallelAnimation = std::make_unique<ParallelAnimation>(renderer);

    duration = 4;
    auto baseSpriteAnimation =
        std::make_unique<SpriteAnimation>(renderer, "resources/Graphics/Pictures/introbase.png", duration);
    baseSpriteAnimation->setRect([](SDL_Surface* surface, RenderSizes rs) {
        SDL_Rect dstRect;

        int dstWidth  = 1.5 * surface->w * rs.ww / rs.aw;
        int dstHeight = 1.5 * surface->h * rs.wh / rs.ah;

        dstRect.x = (rs.ww - dstWidth) / 2;
        dstRect.y = (rs.wh - dstHeight) / 2;
        dstRect.w = dstWidth;
        dstRect.h = dstHeight;

        return dstRect;
    });
    SDL_Point startOffset = {-300, 0};
    SDL_Point endOffset   = {0, 0};
    auto      moveEffect  = std::make_unique<MoveEffect>(renderer,
                                                   baseSpriteAnimation->texture,
                                                   baseSpriteAnimation->surface,
                                                   startOffset,
                                                   endOffset);
    baseSpriteAnimation->addEffect(std::move(moveEffect));

    duration = 5;
    auto oakSpriteAnimation =
        std::make_unique<SpriteAnimation>(renderer, "resources/Graphics/Pictures/introOak.png", duration);
    oakSpriteAnimation->setRect([](SDL_Surface* surface, RenderSizes rs) {
        SDL_Rect dstRect;

        int dstWidth  = surface->w * rs.ww / rs.aw;
        int dstHeight = surface->h * rs.wh / rs.ah;

        dstRect.x = (rs.ww - dstWidth + 40 * rs.ww / rs.aw) / 2;
        dstRect.y = (rs.wh - dstHeight - 140 * rs.wh / rs.ah) / 2;
        dstRect.w = dstWidth;
        dstRect.h = dstHeight;

        return dstRect;
    });
    SDL_Color startColor  = {0, 0, 0, 255};
    SDL_Color endColor    = {255, 255, 255, 255};
    auto      colorEffect = std::make_unique<ColorEffect>(renderer,
                                                     oakSpriteAnimation->texture,
                                                     oakSpriteAnimation->surface,
                                                     startColor,
                                                     endColor);
    oakSpriteAnimation->addEffect(std::move(colorEffect));

    parallelAnimation->addAnimation(std::move(baseSpriteAnimation));
    parallelAnimation->addAnimation(std::move(oakSpriteAnimation));

    animations.push_back(std::move(parallelAnimation));
}
