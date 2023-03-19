#include "gametime.h"

GameTime::GameTime()
{
}

void GameTime::incrementTicks()
{
    ticks += 1;
    if (ticks > MinutesInTicks)
    {
        ticks = 0;
        incrementMinutes();
    }
}

void GameTime::incrementMinutes()
{
    minutes = (minutes + 1) % (60 * 24);
}

std::string GameTime::toString() const
{
    return std::to_string((minutes / 60) % 24) + ":" + std::to_string(minutes % 60);
}

bool GameTime::day() const
{
    return minutes > (7 * 60) && minutes < (21 * 60);
}

bool GameTime::night() const
{
    return !day();
}
