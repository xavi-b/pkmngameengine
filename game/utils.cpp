#include "utils.h"

#include <experimental/random>

std::ostream& operator<<(std::ostream& o, SDL_Rect r)
{
    o << "{(" << r.x << "," << r.y << "), w = " << r.w << " h = " << r.h << "}";
    return o;
}

size_t randint(size_t min, size_t max)
{
    return std::experimental::randint(min, max);
}
