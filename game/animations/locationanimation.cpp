#include "locationanimation.h"

#include "renderutils.h"
#include "textspeech.h"

#include <SDL_image.h>

LocationAnimation::LocationAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
}

LocationAnimation::~LocationAnimation()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
}

void LocationAnimation::reset()
{
    Animation::reset();
    finished = false;
}

void LocationAnimation::restart()
{
    if (isRunning())
        toRestart = true;
    else
        Animation::restart();
}

void LocationAnimation::incrementTicks()
{
    if (!isRunning())
        return;

    if (toRestart && nextText == text)
        toRestart = false;

    if (toRestart && (accumulatedTicks > ticksDuration || Animation::isFinished()))
    {
        toRestart = false;
        text      = nextText;
        nextText  = "";
        reset();
    }
    else
    {
        Animation::incrementTicks();
    }
}

bool LocationAnimation::isFinished() const
{
    if (toRestart)
        return false;

    if (accumulatedTicks > ticksDuration)
        return true;

    return Animation::isFinished();
}

void LocationAnimation::draw(Fps const* fps, RenderSizes rs)
{
    float durationPercentage = (accumulatedTicks + fps->tickPercentage()) / ticksDuration;
    if (durationPercentage > 1.0)
        durationPercentage = 1.0;

    float sinus = 0;

    if (durationPercentage < 0.25)
    {
        durationPercentage *= 2;
        durationPercentage -= 0.5;
        sinus = (std::cos(2 * std::numbers::pi * durationPercentage) - 1) / 2;
    }
    else if (durationPercentage > 0.75)
    {
        durationPercentage -= 0.75;
        durationPercentage *= 2;
        sinus = (std::cos(2 * std::numbers::pi * durationPercentage) - 1) / 2;
    }

    int borderSize = TextSpeech::TextBoxBorderSize;

    SDL_Rect rect;
    rect.w = rs.ww / 3;
    rect.h = 1.5 * TextSpeech::TextBoxHeight * rs.wh / rs.ah;
    rect.x = 0;
    rect.y = sinus * rect.h;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int fontSize = RenderUtils::TextSize;

    rect.y += (rect.h - (fontSize + 10) * rs.wh / rs.ah) / 2;

    RenderUtils::drawGreyTextCentered(renderer, rs, text, fontSize, rect);
}

void LocationAnimation::restart(std::string const& location)
{
    if (!isRunning())
        text = location;
    else
        nextText = location;
    restart();
}
