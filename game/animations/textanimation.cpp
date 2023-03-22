#include "textanimation.h"

TextAnimation::TextAnimation(SDL_Renderer* renderer, const std::string& text) : Animation(renderer), text(text)
{
}

void TextAnimation::incrementTicks()
{
    ticks++;
    if (ticks * NbCharPerTick > text.size())
        stop();
}

void TextAnimation::draw(const Fps* /*fps*/, RenderSizes /*rs*/)
{
}

void TextAnimation::forceEnd()
{
    ticks   = text.size() / NbCharPerTick + 1;
    running = false;
}

std::string TextAnimation::currentText() const
{
    return text.substr(0, ticks * NbCharPerTick);
}
