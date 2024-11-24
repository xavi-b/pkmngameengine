#ifndef MAPSCENE_H
#define MAPSCENE_H

#include "animations/battleintroanimation.h"
#include "animations/fadeanimation.h"
#include "animations/map/dooranimation.h"
#include "animations/map/ledgeanimation.h"
#include "animations/map/mapanimation.h"
#include "animations/map/stairsanimation.h"
#include "animations/weather/flashanimation.h"
#include "animations/weather/weatheranimation.h"
#include "entity.h"
#include "map.h"
#include "menu.h"
#include "pkmn.h"
#include "scene.h"
#include "sprite.h"
#include "utils.h"

#include <SDL_image.h>
#include <map>
#include <memory>
#include <string>

class MapScene : public Scene
{
public:
    static constexpr int TilePixelSize     = 32;
    static constexpr int EntityPixelHeight = 48;

    MapScene(SDL_Renderer* renderer, std::string const& mapPath);
    ~MapScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;
    void         drawPlayer(Fps const* fps, RenderSizes rs, SDL_Rect dstPlayerRect);
    void         drawTile(Fps const*                                   fps,
                          RenderSizes                                  rs,
                          Tile::TilePtr const&                         tile,
                          size_t                                       i,
                          size_t                                       j,
                          std::pair<SDL_Surface*, SDL_Texture*> const& sprite,
                          SDL_Rect                                     srcRect,
                          SDL_Rect                                     dstRect);

    virtual void drawAmbientOverlay(Fps const* fps, RenderSizes rs, size_t offsetX, size_t offsetY);
    virtual void drawWeather(Fps const* fps, RenderSizes rs);
    virtual void drawFlashDarkness(Fps const* fps, RenderSizes rs);

    virtual void    initPlayerPosition(size_t            x,
                                       size_t            y,
                                       size_t            l,
                                       Entity::Direction direction = Entity::Direction::NONE);
    virtual void    initMovingPlayerPosition(size_t x, size_t y, size_t l, Entity::Direction direction);
    void            initClosingDoor(size_t x, size_t y);
    void            initStairsEntrance(StairsAnimation::Direction direction);
    void            startLedgeJump(Entity& entity);
    void            incrementLedgeJump(Entity& entity);
    void            finishLedgeJump(Entity& entity);
    void            stop(Entity& entity);
    virtual void    move(Entity& entity, bool force = false);
    virtual Entity* entityAt(size_t x, size_t y, size_t l) const;
    virtual Entity* entityPreviousAt(size_t x, size_t y, size_t l) const;
    virtual bool    canMove(Entity const& entity, size_t x, size_t y, size_t l, bool force) const;

    Entity* facedEntity(Entity const& entity) const;
    bool    isEntityFacingTile(Entity const& entity, std::function<bool(size_t x, size_t y, size_t l)> func) const;
    bool    isNormalTile(size_t x, size_t y, size_t l) const;
    bool    isWaterTile(size_t x, size_t y, size_t l) const;
    bool    isEntityFacingWaterTile(Entity const& entity) const;
    bool    isTreeTile(size_t x, size_t y, size_t l) const;
    bool    isEntityFacingTreeTile(Entity const& entity) const;
    bool    isRockTile(size_t x, size_t y, size_t l) const;
    bool    isEntityFacingRockTile(Entity const& entity) const;
    bool    isWaterfallTile(size_t x, size_t y, size_t l) const;
    bool    isEntityFacingWaterfallTile(Entity const& entity) const;
    bool    isGrassTile(size_t x, size_t y, size_t l) const;
    bool    isTallGrassTile(size_t x, size_t y, size_t l) const;
    bool    isUnderWaterGrassTile(size_t x, size_t y, size_t l) const;
    bool    isLedgeTile(size_t x, size_t y, size_t l) const;
    bool    isLedgePassable(Entity const& entity, size_t x, size_t y, size_t l) const;

    Event* eventAt(size_t x, size_t y, size_t l) const;
    Event* facedEvent(Entity const& entity) const;
    Event* facedPreviousEvent(Entity const& entity) const;

    bool isPlayerEntity(Entity const* entity) const;
    bool isPlayerEntity(Entity const& entity) const;

    virtual bool                                  manageEvents();
    virtual bool                                  manageEncounters();
    virtual std::unique_ptr<BattleIntroAnimation> manageBattleIntro();

    virtual bool pushScene() const override;
    virtual void popReset() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    std::tuple<size_t, size_t, size_t> currentPlayerPosition() const;

    void changeWeather(Map::Weather weather);

    virtual bool shouldShowNightTextures() const = 0;

    bool turnOnFlash();

protected:
    std::map<std::string, std::pair<SDL_Surface*, SDL_Texture*>> sprites;
    std::map<std::string, std::pair<SDL_Surface*, SDL_Texture*>> lightsSprites;

    std::unique_ptr<Map>                                       map;
    std::unique_ptr<Sprite>                                    playerSprite;
    std::unique_ptr<Sprite>                                    playerRunSprite;
    std::unique_ptr<Sprite>                                    playerSurfSprite;
    std::unique_ptr<Sprite>                                    surfSprite;
    std::unique_ptr<Sprite>                                    divingSprite;
    Entity::Direction                                          playerSpriteInitialDirection = Entity::Direction::NONE;
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
    std::unique_ptr<FlashAnimation>                              flashAnimation;
    std::unique_ptr<StairsAnimation>                             stairsExitAnimation;
    std::pair<int, int>                                          stairsExitPosition;
    std::unique_ptr<StairsAnimation>                             stairsEntranceAnimation;
    std::unique_ptr<LedgeAnimation>                              ledgeAnimation;

    bool                  openMenu = false;
    std::unique_ptr<Menu> menu;
    bool                  openPkmns     = false;
    bool                  openBag       = false;
    bool                  flash         = true;
    bool                  diving        = false;
    bool                  preventInputs = false;

    std::string goToScene = "";

private:
    size_t getPlayerOffsetX(Fps const* fps, RenderSizes rs) const;
    size_t getPlayerOffsetY(Fps const* fps, RenderSizes rs) const;
    void   drawGrass(Fps const* fps, RenderSizes rs, size_t x, size_t y, size_t l);
    void   drawTallGrass(SDL_Rect dstRect,
                         size_t   x,
                         size_t   y,
                         size_t   l,
                         int      entityOffsetX,
                         int      entityOffsetY,
                         int      width,
                         int      height);
    void   tryDrawingHighGrass(Fps const*    fps,
                               RenderSizes   rs,
                               Entity const& entity,
                               Sprite const& sprite,
                               SDL_Rect      dstRect);
};

#endif // MAP_H
