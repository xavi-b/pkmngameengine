#include "introanimation.h"

#include "spriteanimation.h"
#include "parallelanimation.h"
#include "sequentialanimation.h"

IntroAnimation::IntroAnimation(SDL_Renderer* renderer) : SequentialAnimation(renderer)
{
    std::string spritePath;
    SDL_Rect    startRect;
    SDL_Rect    endRect;
    int         duration;

    /*
    int dstWidth  = surface->w * rs.ww / rs.aw;
    int dstHeight = surface->h * rs.wh / rs.ah;

    dstRect.x = (rs.ww - dstWidth) / 2;
    dstRect.y = (rs.wh - dstHeight) / 2;
    */

    auto parallelAnimation = std::make_unique<SpriteAnimation>(renderer, spritePath, duration);

    duration = 10;
    auto baseSpriteAnimation =
        std::make_unique<SpriteAnimation>(renderer, "resources/Graphics/Pictures/introbase.png", duration);
    startRect = {0, 0, 10, 10};
    endRect   = {0, 0, 10, 10};

    duration = 10;
    auto oakSpriteAnimation =
        std::make_unique<SpriteAnimation>(renderer, "resources/Graphics/Pictures/introOak.png", duration);
    startRect = {0, 0, 10, 10};
    endRect   = startRect;

    animations.push_back(std::move(parallelAnimation));
}
