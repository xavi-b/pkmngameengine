#include "textanimation.h"

#include <iostream>

TextAnimation::TextAnimation(SDL_Renderer* renderer, const std::string& text) : Animation(renderer), text(text)
{
}

void TextAnimation::start()
{
    Animation::start();
}

void TextAnimation::stop()
{
    Animation::stop();
}

void TextAnimation::incrementTicks()
{
    if (!isStarted() || isFinished())
        return;

    ticksPercentage++;
    if (ticksPercentage * NbCharPerTick > text.size())
        forceEnd();
}

void TextAnimation::draw(const Fps* /*fps*/, RenderSizes /*rs*/)
{
}

void TextAnimation::forceEnd()
{
    ticksPercentage = text.size() / NbCharPerTick + 1;
    Animation::forceEnd();
}

std::string TextAnimation::currentText() const
{
    return text.substr(0, ticksPercentage * NbCharPerTick);
}
