#include "introanimation.h"

#include "spriteanimation.h"

IntroAnimation::IntroAnimation(SDL_Renderer* renderer) : SequentialAnimation(renderer)
{
    //    baseSurface = IMG_Load("resources/Graphics/Pictures/introbase.png");
    //    baseTexture = SDL_CreateTextureFromSurface(renderer, baseSurface);

    std::string spritePath;
    SDL_Rect    startRect;
    SDL_Rect    endRect;
    int         duration;

    // TODO sprites
    spritePath = "";
    startRect;
    endRect;
    duration;
    animations.push_back(std::make_unique<SpriteAnimation>(renderer, spritePath, duration));
}
