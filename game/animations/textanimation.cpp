#include "textanimation.h"

#include <climits>

TextAnimation::TextAnimation(SDL_Renderer* renderer, const std::string& text) : renderer(renderer), text(text)
{
}

void TextAnimation::incrementTicks()
{
    currentFrame++;
    if (currentFrame * NbCharPerTick > text.size())
        stop();
}

std::string TextAnimation::currentText() const
{
    return text.substr(0, currentFrame * NbCharPerTick);
}

void TextAnimation::forceEnd()
{
    currentFrame = text.size() / NbCharPerTick + 1;
    running      = false;
}
