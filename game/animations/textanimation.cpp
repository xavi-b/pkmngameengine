#include "textanimation.h"

#include <iostream>

TextAnimation::TextAnimation(SDL_Renderer* renderer, std::string const& text) : Animation(renderer), text(text)
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

    accumulatedTicks++;
    if (accumulatedTicks * NbCharPerTick > text.size())
        forceEnd();
}

void TextAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void TextAnimation::forceEnd()
{
    accumulatedTicks = text.size() / NbCharPerTick + 1;
    Animation::forceEnd();
}

std::string TextAnimation::currentText(Fps const* fps) const
{
    return text.substr(0, (accumulatedTicks + fps->tickPercentage()) * NbCharPerTick);
}

void TextAnimation::resetFinished()
{
    finished = false;
}
