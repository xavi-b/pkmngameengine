#include "utils.h"

std::ostream& operator<<(std::ostream& o, SDL_Rect r)
{
    o << "{(" << r.x << "," << r.y << "), w = " << r.w << " h = " << r.h << "}";
    return o;
}
