#ifndef GAMETIME_H
#define GAMETIME_H

#include <string>

class GameTime
{
public:
    GameTime();

    void        incrementTicks();
    std::string toString() const;

    static constexpr int TicksForMinute = 10;

    bool isDay() const;

    short getMinutes() const;
    void  setMinutes(short newMinutes);

private:
    void incrementMinutes();

    int   ticks   = 0;
    short minutes = 0;
};

#endif // GAMETIME_H
