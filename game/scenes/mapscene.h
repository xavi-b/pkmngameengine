#ifndef MAPSCENE_H
#define MAPSCENE_H

#include "animations/battleintroanimation.h"
#include "animations/fadeanimation.h"
#include "animations/map/dooranimation.h"
#include "animations/map/mapanimation.h"
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
    virtual void    initMovingPlayerPosition(int x, int y, Entity::Direction direction);
    void            initClosingDoor(int x, int y);
    void            stop(Entity& entity);
    virtual void    move(Entity& entity, bool force = false);
    virtual Entity* entityAt(size_t x, size_t y, size_t l) const;
    virtual Entity* entityPreviousAt(size_t x, size_t y, size_t l) const;
    virtual bool    canMove(size_t x, size_t y, size_t l, bool force) const;

    virtual bool                                  manageEvents();
    virtual bool                                  manageEncounters();
    virtual std::unique_ptr<BattleIntroAnimation> manageBattleIntro();

    virtual bool pushScene() const override;
    virtual void popReset() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    std::pair<size_t, size_t> currentPlayerPosition() const;

    void changeWeather(Map::Weather weather);

protected:
    std::map<std::string, std::pair<SDL_Surface*, SDL_Texture*>> sprites;

    std::unique_ptr<Map>                                       map;
    std::unique_ptr<Sprite>                                    playerSprite;
    std::map<std::unique_ptr<Entity>, std::unique_ptr<Sprite>> entities;

    Map::Weather weather = Map::Weather::NONE;

    Pkmn::PkmnPtr                                                emptyPkmnPtr = nullptr;
    Pkmn::PkmnPtr                                                encounteredPkmn;
    std::unique_ptr<BattleIntroAnimation>                        battleIntro;
    std::unique_ptr<FadeAnimation>                               fadeInAnimation;
    std::unique_ptr<FadeAnimation>                               fadeOutAnimation;
    std::unique_ptr<WeatherAnimation>                            weatherAnimation;
    std::map<std::pair<int, int>, std::unique_ptr<MapAnimation>> tilesAnimations;
    std::pair<int, int>                                          doorOpeningPosition;
    std::unique_ptr<DoorAnimation>                               doorOpeningAnimation;
    std::pair<int, int>                                          doorClosingPosition;
    std::unique_ptr<DoorAnimation>                               doorClosingAnimation;

    bool                  openMenu = false;
    std::unique_ptr<Menu> menu;
    bool                  openPkmns = false;
    bool                  openBag   = false;
    bool                  surfing   = false;

    std::string goToScene = "";
};

#endif // MAP_H
