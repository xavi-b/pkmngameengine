# Support this project

[![](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=DARKFRWPCP2VG)

[<img src="https://www.paypalobjects.com/webstatic/icon/pp32.png" /> PayPal Me](https://www.paypal.me/xavibesson)

[<img src="https://github.githubassets.com/assets/pinned-octocat-093da3e6fa40.svg" width="32" /> GitHub Sponsors](https://github.com/sponsors/xavi-b)

[<img src="https://c5.patreon.com/external/favicon/favicon-masked.svg" width="32" /> Patreon](https://www.patreon.com/xavibesson)

# Requirements
* boost_json
* qt6
* sdl2
* sdl2-image
* sdl2-mixer
* sdl2-ttf

## Linux

Install the above requirements using your package manager

## Windows

* Download boost source
    * Build by calling `bootstrap.bat` and then `b2 link=static`
* Download the latest SDL 2 libraries and put them in the same folder:
    * https://github.com/libsdl-org/SDL/releases
    * https://github.com/libsdl-org/SDL_ttf/releases
    * https://github.com/libsdl-org/SDL_mixer/releases
    * https://github.com/libsdl-org/SDL_image/releases
* Update your CMAKE_PREFIX_PATH variable:
    * CMAKE_PREFIX_PATH=%{Qt:QT_INSTALL_PREFIX};C:\SDL\cmake;C:\boost\boost_1_82_0\stage\lib\cmake\Boost-1.82.0;

## Resources
* Audio, Fonts and Graphics from Pokemon Essentials 21.1
    * Put the mentioned folders in game/resources

# Thanks to
* Pokemon stats and logic from Bulbapedia
    * https://bulbapedia.bulbagarden.net/wiki/Main_Page
* PkmnEditor inspired from Essentials PBS Editor
    * https://github.com/Marin-MK/PBS-Editor
