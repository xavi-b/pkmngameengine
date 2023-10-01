#include "utils.h"

#include <experimental/random>

std::ostream& operator<<(std::ostream& o, SDL_Rect r)
{
    o << "{(" << r.x << "," << r.y << "), w = " << r.w << " h = " << r.h << "}";
    return o;
}

namespace Utils
{

size_t randint(size_t min, size_t max)
{
    return std::experimental::randint(min, max);
}

std::string dataDir()
{
#ifdef LINUX
    return std::getenv("HOME") + std::string("/.local/share/") + PROJECT_NAME "/";
#elif WINDOWS
    return std::getenv("HOMEPATH") + std::string("/AppData/Roaming/") + PROJECT_NAME "/";
#else
    return "./"
#endif
}

std::string configDir()
{
#ifdef LINUX
    return std::getenv("HOME") + std::string("/.config/") + PROJECT_NAME "/";
#elif WINDOWS
    return std::getenv("HOMEPATH") + std::string("/AppData/Roaming/") + PROJECT_NAME "/";
#else
    return "./"
#endif
}

} // namespace Utils
