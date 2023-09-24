#include "settings.h"

#include "utils.h"

#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>

namespace fs = std::filesystem;

Settings* Settings::instance()
{
    static Settings i;
    return &i;
}

bool Settings::musicOn()
{
    return pt.get<bool>("General.musicOn", false);
}

bool Settings::savedGame()
{
    // TODO
    return pt.get<bool>("General.savedGame", true);
}

bool Settings::autoText()
{
    return pt.get<bool>("General.autoText", false);
}

Settings::Settings()
{
    std::string appName = PROJECT_NAME;

#ifdef LINUX
    std::string configPathName = std::getenv("HOME") + std::string("/.config/") + appName;
#elif WINDOWS
    std::string configPathName = std::getenv("HOMEPATH") + std::string("/AppData/Roaming/") + appName;
#else
    std::string configPathName = "."
#endif

    fs::path configPath(configPathName);
    fs::create_directories(configPath);

    configPath = configPath.append("settings.ini");

    if (!fs::exists(configPath))
        std::fstream file(configPath, file.out);

    boost::property_tree::ini_parser::read_ini(configPath, pt);
}

Settings::~Settings()
{
    // TODO save
}
