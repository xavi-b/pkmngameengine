#ifndef TEXTANIMATION_H
#define TEXTANIMATION_H

#include "animation.h"

class TextAnimation : public Animation
{
public:
    TextAnimation(SDL_Renderer* renderer, const std::string& text);

    std::string text;

private:
    SDL_Renderer* renderer;
};

#endif // TEXTANIMATION_H
