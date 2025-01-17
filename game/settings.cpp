#include "settings.h"

#include "utils.h"

#include <boost/property_tree/ini_parser.hpp>

Settings* Settings::instance()
{
    static Settings i;
    return &i;
}

bool Settings::musicOn()
{
    return pt.get<bool>("General.musicOn", false);
}

void Settings::setSavedGame(bool b)
{
    pt.put("General.savedGame", b);
    save();
}

bool Settings::savedGame()
{
    return pt.get<bool>("General.savedGame", false);
}

bool Settings::autoText()
{
    return pt.get<bool>("General.autoText", false);
}

Settings::Settings()
{
    configPath = Utils::configDir();

    fs::create_directories(configPath);

    configPath = configPath.append("settings.ini");

    if (!fs::exists(configPath))
        std::fstream file(configPath, std::fstream::out);

    pt::ini_parser::read_ini(configPath.string(), pt);
}

Settings::~Settings()
{
}

void Settings::save()
{
    pt::ini_parser::write_ini(configPath.string(), pt);
}
