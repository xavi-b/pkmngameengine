#include "utils.h"

#ifdef WINDOWS
#include <random>
namespace random {
    size_t randint(int min, int max) {
        return rand() % (max + 1 - min) + min;
    }
}

std::string envvar(std::string const& str) {
    std::string res;
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, str.c_str()) == 0 && buf != nullptr)
    {
        res = buf;
    }
    return res;
}
#else
#include <experimental/random>
namespace random = std::experimental;
#endif

std::ostream& operator<<(std::ostream& o, SDL_Rect r)
{
    o << "{(" << r.x << "," << r.y << "), w = " << r.w << " h = " << r.h << "}";
    return o;
}

namespace Utils
{

size_t randint(int min, int max)
{
    return random::randint(min, max);
}

size_t randuint(size_t min, size_t max)
{
    return random::randint(min, max);
}

std::string dataDir()
{
#if defined(LINUX)
    return std::getenv("HOME") + std::string("/.local/share/") + PROJECT_NAME "/";
#elif defined(WINDOWS)
    return envvar("HOMEPATH") + std::string("/AppData/Roaming/") + PROJECT_NAME "/";
#else
    return "./"
#endif
}

std::string configDir()
{
#if defined(LINUX)
    return std::getenv("HOME") + std::string("/.config/") + PROJECT_NAME "/";
#elif defined(WINDOWS)
    return envvar("HOMEPATH") + std::string("/AppData/Roaming/") + PROJECT_NAME "/";
#else
    return "./"
#endif
}

} // namespace Utils
