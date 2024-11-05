#ifndef MAPSCENE_H
#define MAPSCENE_H

#include "animations/battleintroanimation.h"
#include "animations/fadeanimation.h"
#include "entity.h"
#include "map.h"
#include "menu.h"
#include "pkmn.h"
#include "scene.h"
#include "scenes/encounterscene.h"
#include "sprite.h"
#include "utils.h"

#include <SDL_image.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class MapScene : public Scene
{
public:
    static constexpr int TilePixelSize     = 32;
    static constexpr int EntityPixelHeight = 48;

    MapScene(SDL_Renderer* renderer, std::string const& mapPath);
    ~MapScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual void drawWeather(Fps const* fps, RenderSizes rs);
    virtual void drawNight();

    virtual void    initPlayerPosition(int x, int y, Entity::Direction direction = Entity::Direction::NONE);
    virtual void    move(Entity& entity);
    virtual Entity* entityAt(size_t x, size_t y, size_t l);

    virtual bool                                  manageEvents();
    virtual bool                                  manageEncounters();
    virtual std::unique_ptr<BattleIntroAnimation> manageBattleIntro();

    virtual bool pushScene() const override;
    virtual void popReset() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    std::pair<size_t, size_t> currentPlayerPosition() const;

    virtual bool entitiesShouldFreeze() const;

    void changeWeather(Map::Weather weather);

protected:
    std::map<std::string, std::pair<SDL_Surface*, SDL_Texture*>> sprites;

    std::unique_ptr<Map>                                       map;
    std::unique_ptr<Sprite>                                    playerSprite;
    std::map<std::unique_ptr<Entity>, std::unique_ptr<Sprite>> entities;

    Map::Weather weather = Map::Weather::NONE;

    Pkmn::PkmnPtr                         emptyPkmnPtr = nullptr;
    Pkmn::PkmnPtr                         encounteredPkmn;
    std::unique_ptr<BattleIntroAnimation> battleIntro;
    std::unique_ptr<FadeAnimation>        fadeInAnimation;
    std::unique_ptr<FadeAnimation>        fadeOutAnimation;
    std::unique_ptr<WeatherAnimation>     weatherAnimation;

    bool                  openMenu = false;
    std::unique_ptr<Menu> menu;
    bool                  openPkmns = false;
    bool                  openBag   = false;

    std::string goToScene = "";
};

#endif // MAP_H
