#ifndef PKMN_H
#define PKMN_H

#include <array>
#include <vector>

struct Attack
{

};

struct Type
{

};

class Pkmn
{
public:
    Pkmn();

private:
    std::array<Attack, 4> attacks;
    std::vector<Type> types;
};

#endif // PKMN_H
