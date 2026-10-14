#include "textanimation.h"

#include <iostream>

namespace
{

size_t codePointCount(std::string const& text)
{
    size_t count = 0;
    for (unsigned char byte : text)
    {
        if ((byte & 0xC0) != 0x80)
            ++count;
    }
    return count;
}

// Byte length of the first codePointCount code points, never splitting a UTF-8 sequence.
size_t prefixBytes(std::string const& text, size_t codePoints)
{
    size_t bytes = 0;
    size_t seen  = 0;
    while (bytes < text.size() && seen < codePoints)
    {
        unsigned char byte = static_cast<unsigned char>(text[bytes]);
        size_t        len  = 1;
        if ((byte & 0xE0) == 0xC0)
            len = 2;
        else if ((byte & 0xF0) == 0xE0)
            len = 3;
        else if ((byte & 0xF8) == 0xF0)
            len = 4;

        if (bytes + len > text.size())
            break;
        bytes += len;
        ++seen;
    }
    return bytes;
}

} // namespace

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
    if (accumulatedTicks * NbCharPerTick > codePointCount(text))
        forceEnd();
}

void TextAnimation::draw(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void TextAnimation::forceEnd()
{
    accumulatedTicks = codePointCount(text) / NbCharPerTick + 1;
    Animation::forceEnd();
}

std::string TextAnimation::currentText(Fps const* fps) const
{
    size_t count = static_cast<size_t>((accumulatedTicks + fps->tickPercentage()) * NbCharPerTick);
    return text.substr(0, prefixBytes(text, count));
}

void TextAnimation::resetFinished()
{
    finished = false;
}
