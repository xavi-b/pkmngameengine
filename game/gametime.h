#ifndef GAMETIME_H
#define GAMETIME_H

#include <string>

class GameTime
{
public:
    GameTime();

    void        incrementTicks();
    std::string toString() const;

    static constexpr int MinutesInTicks = 4;

    bool day() const;
    bool night() const;

private:
    void incrementMinutes();

    int   ticks   = 0;
    short minutes = 0;
};

#endif // GAMETIME_H
