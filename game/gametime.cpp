#include "gametime.h"

#include "game.h"

#include <iostream>

GameTime::GameTime()
{
}

void GameTime::incrementTicks()
{
    ticks += 1;
    if (ticks > TicksForMinute)
    {
        ticks = 0;
        incrementMinutes();
    }
}

short GameTime::getMinutes() const
{
    return minutes;
}

void GameTime::setMinutes(short newMinutes)
{
    minutes = newMinutes;
}

std::string GameTime::toString() const
{
    short h = (minutes / 60) % 24;
    short m = minutes % 60;
    return (h >= 10 ? "" : "0") + std::to_string(h) + ":" + (m >= 10 ? "" : "0") + std::to_string(m);
}

bool GameTime::isDay() const
{
    return minutes > (7 * 60) && minutes < (21 * 60);
}

void GameTime::incrementMinutes()
{
    minutes = (minutes + 1) % (60 * 24);

    if (Game::instance()->isDebug())
        std::cout << "Current in game time: " << toString() << std::endl;
}
