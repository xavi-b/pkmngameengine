#ifndef SETTINGS_H
#define SETTINGS_H

#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <string>

namespace pt = boost::property_tree;
namespace fs = std::filesystem;

class Settings
{
public:
    static Settings* instance();

    bool musicOn();
    void setSavedGame(bool b);
    bool savedGame();
    bool autoText();

private:
    Settings();
    ~Settings();

    void save();

    pt::ptree pt;
    fs::path  configPath;
};

#endif // SETTINGS_H
