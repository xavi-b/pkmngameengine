#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <boost/property_tree/ptree.hpp>

class Settings
{
public:
    static Settings* instance();

    bool musicOn();
    bool savedGame();

private:
    Settings();

    boost::property_tree::ptree pt;
};

#endif // SETTINGS_H
