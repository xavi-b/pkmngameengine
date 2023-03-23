#ifndef DEFINES_H
#define DEFINES_H

#if defined(__linux) || defined(__linux__) || defined(linux)
#define LINUX
#elif defined(__APPLE__)
#define MACOS
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(_WIN64)
#define WINDOWS
#endif

#include <iostream>
#include <SDL.h>
#include <boost/locale.hpp>

namespace lc = boost::locale;

std::ostream& operator<<(std::ostream& o, SDL_Rect r);

#endif // DEFINES_H
