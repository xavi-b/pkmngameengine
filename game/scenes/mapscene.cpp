#include "mapscene.h"

#include "animations/map/grassanimation.h"
#include "animations/map/groundanimation.h"
#include "animations/map/underwatergrassanimation.h"
#include "animations/weather/rainanimation.h"
#include "bagscene.h"
#include "entities/erasableentity.h"
#include "game.h"
#include "pkmnsscene.h"
#include "scenes/encounterscene.h"
#include "sprites/erasablesprite.h"
#include "sprites/squaresprite.h"
#include "sprites/surfsprite.h"

#include <fstream>
#include <iostream>
#include <numbers>
#include <random>
#include <sstream>

MapScene::MapScene(SDL_Renderer* renderer, std::string const& mapPath) : Scene(renderer)
{
    std::ifstream     t(mapPath);
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto mapPtr = js::value_to<std::unique_ptr<Map>>(js::parse(buffer.str()));
    map.swap(mapPtr);

    menu = std::make_unique<Menu>(renderer);
    menu->init();
}

MapScene::~MapScene()
{
    for (auto const& [key, value] : sprites)
    {
        auto [surface, texture] = value;

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

void MapScene::init()
{
    playerSprite     = std::make_unique<Sprite>(renderer);
    playerSurfSprite = std::make_unique<Sprite>(renderer);
    surfSprite       = std::make_unique<SurfSprite>(renderer);
    divingSprite     = std::make_unique<SurfSprite>(renderer);
    if (Game::instance()->data.player.getGender() == Player::Gender::BOY)
    {
        playerSprite->load("resources/Graphics/Characters/boy_run.png", shouldShowNightTextures());
        playerSurfSprite->load("resources/Graphics/Characters/boy_surf.png", diving || shouldShowNightTextures());
    }
    else
    {
        playerSprite->load("resources/Graphics/Characters/girl_run.png", shouldShowNightTextures());
        playerSurfSprite->load("resources/Graphics/Characters/girl_surf.png", diving || shouldShowNightTextures());
    }
    surfSprite->load("resources/Graphics/Characters/base_surf.png", shouldShowNightTextures());
    divingSprite->load("resources/Graphics/Characters/base_dive.png", diving || shouldShowNightTextures());
    playerSprite->forceSpriteDirection(playerSpriteInitialDirection);
    playerSurfSprite->forceSpriteDirection(playerSpriteInitialDirection);
    surfSprite->forceSpriteDirection(playerSpriteInitialDirection);
    divingSprite->forceSpriteDirection(playerSpriteInitialDirection);

    for (size_t l = 0; l < map->getLevels().size(); ++l)
    {
        auto& level = map->getLevels()[l];
        auto& layer = level->getSpecialTileLayer();
        for (size_t j = 0; j < map->getNRow(); ++j)
        {
            for (size_t i = 0; i < map->getNCol(); ++i)
            {
                auto& tile = (*layer.get())(i, j);
                if (tile)
                {
                    if (*tile.get() == SpecialTileType::TREE)
                    {
                        auto entity       = std::make_unique<ErasableEntity>();
                        entity->x         = i;
                        entity->y         = j;
                        entity->previousX = i;
                        entity->previousY = j;
                        auto entitySprite = std::make_unique<ErasableSprite>(renderer);
                        entitySprite->load("resources/Graphics/Characters/Object tree 1.png",
                                           shouldShowNightTextures());
                        entities.emplace(std::move(entity), std::move(entitySprite));
                    }
                    else if (*tile.get() == SpecialTileType::ROCK)
                    {
                        auto entity       = std::make_unique<ErasableEntity>();
                        entity->x         = i;
                        entity->y         = j;
                        entity->previousX = i;
                        entity->previousY = j;
                        auto entitySprite = std::make_unique<ErasableSprite>(renderer);
                        entitySprite->load("resources/Graphics/Characters/Object rock.png", shouldShowNightTextures());
                        entities.emplace(std::move(entity), std::move(entitySprite));
                    }
                    else if (*tile.get() == SpecialTileType::BOULDER)
                    {
                        auto entity       = std::make_unique<Entity>();
                        entity->x         = i;
                        entity->y         = j;
                        entity->previousX = i;
                        entity->previousY = j;
                        entity->boulder   = true;
                        auto entitySprite = std::make_unique<SquareSprite>(renderer);
                        entitySprite->load("resources/Graphics/Characters/Object boulder.png",
                                           shouldShowNightTextures());
                        entities.emplace(std::move(entity), std::move(entitySprite));
                    }
                }
            }
        }
    }

    fadeOutAnimation = std::make_unique<FadeAnimation>(renderer, false);
    fadeInAnimation  = std::make_unique<FadeAnimation>(renderer, true);
    fadeInAnimation->reset();
    fadeInAnimation->start();

    flashAnimation = std::make_unique<FlashAnimation>(renderer);
}

void MapScene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    player.previousSpeed = player.speed;

    playerSprite->setAccumulatedTicks((playerSprite->getAccumulatedTicks() + 1) % player.speed);
    playerSurfSprite->setAccumulatedTicks((playerSprite->getAccumulatedTicks() + 1) % player.speed);
    surfSprite->setAccumulatedTicks((playerSprite->getAccumulatedTicks() + 1) % player.speed);
    divingSprite->setAccumulatedTicks((playerSprite->getAccumulatedTicks() + 1) % player.speed);

    if (player.direction == Entity::Direction::NONE && player.speed == Entity::Speed::WALK)
        playerSprite->setAccumulatedTicks(0);

    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
        auto entity = it->first.get();
        auto sprite = it->second.get();

        sprite->setAccumulatedTicks((sprite->getAccumulatedTicks() + 1) % entity->speed);

        if (entity->direction == Entity::Direction::NONE && entity->speed == Entity::Speed::WALK)
            sprite->setAccumulatedTicks(0);

        if (entity->boulder)
        {
            if (sprite->getAccumulatedTicks() == 0)
            {
                stop(*entity);
            }
        }

        auto erasableEntity = dynamic_cast<ErasableEntity*>(entity);
        if (erasableEntity)
        {
            sprite->setAccumulatedTicks(erasableEntity->eraseTicks());
            erasableEntity->incrementEraseTicks();
            if (erasableEntity->erased())
                it = entities.erase(it);
        }
    }

    if (flashAnimation->isRunning())
    {
        flashAnimation->incrementTicks();
    }

    if (weatherAnimation)
    {
        if (!weatherAnimation->isStarted())
            weatherAnimation->start();

        weatherAnimation->incrementTicks();
    }

    if (fadeInAnimation->isRunning())
    {
        fadeInAnimation->incrementTicks();
    }

    if (fadeInAnimation->isFinished())
    {
        if (doorClosingAnimation && !doorClosingAnimation->isStarted())
        {
            doorClosingAnimation->start();
        }
    }

    if (fadeOutAnimation->isRunning())
    {
        fadeOutAnimation->incrementTicks();
        preventInputs = true;
        return;
    }

    if (doorOpeningAnimation && doorOpeningAnimation->isRunning())
    {
        doorOpeningAnimation->incrementTicks();
        preventInputs = true;
        return;
    }

    if (doorClosingAnimation && doorClosingAnimation->isRunning())
    {
        doorClosingAnimation->incrementTicks();
    }

    if (stairsExitAnimation && stairsExitAnimation->isRunning())
    {
        stairsExitAnimation->incrementTicks();
        preventInputs = true;

        if (!fadeOutAnimation->isStarted())
        {
            fadeOutAnimation->reset();
            fadeOutAnimation->start();
        }

        return;
    }

    if (stairsEntranceAnimation && stairsEntranceAnimation->isStarted())
    {
        if (!stairsEntranceAnimation->isFinished())
        {
            stairsEntranceAnimation->incrementTicks();
            preventInputs = true;
            return;
        }
        else
        {
            stairsEntranceAnimation.release();
        }
    }

    if (ledgeAnimation && ledgeAnimation->isStarted())
    {
        if (!ledgeAnimation->isFinished())
        {
            if (playerSprite->getAccumulatedTicks() == 0)
                incrementLedgeJump(player);

            ledgeAnimation->incrementTicks();
            preventInputs = true;
            return;
        }
        else
        {
            if (playerSprite->getAccumulatedTicks() == 0)
                finishLedgeJump(player);

            auto it = tilesAnimations.find({player.x, player.y});

            if (it == tilesAnimations.end())
            {
                if (isNormalTile(player.x, player.y, player.l))
                    tilesAnimations[{player.x, player.y}] =
                        std::make_unique<GroundAnimation>(renderer, shouldShowNightTextures());
                else if (isGrassTile(player.x, player.y, player.l))
                    tilesAnimations[{player.x, player.y}] =
                        std::make_unique<GrassAnimation>(renderer, shouldShowNightTextures());
                tilesAnimations[{player.x, player.y}]->start();
            }
            else
            {
                it->second->restart();
            }
        }
    }

    if (battleIntro && !battleIntro->isFinished())
    {
        battleIntro->incrementTicks();
        preventInputs = true;
        return;
    }

    if (openMenu)
    {
        menu->update(inputs);

        if (menu->isFinished())
        {
            if (menu->isSelected())
            {
                switch (menu->selection())
                {
                case Menu::PKMNS: {
                    openPkmns = true;
                    fadeOutAnimation->reset();
                    fadeOutAnimation->start();
                    break;
                }
                case Menu::BAG: {
                    openBag = true;
                    fadeOutAnimation->reset();
                    fadeOutAnimation->start();
                    break;
                }
                case Menu::SAVE: {
                    Game::instance()->save();
                    break;
                }
                case Menu::QUIT: {
                    Game::instance()->quit();
                    break;
                }
                default:
                    break;
                }
            }
            else
            {
                openMenu = false;
            }

            menu->reset();
        }

        preventInputs = true;
        return;
    }

    for (auto it = tilesAnimations.begin(); it != tilesAnimations.end(); ++it)
    {
        auto& anim = *(it->second.get());
        anim.incrementTicks();
    }

    if (playerSprite->getAccumulatedTicks() != 0)
        return;

    bool event     = manageEvents();
    bool encounter = manageEncounters();

    if (event)
    {
        stop(player);
        preventInputs = true;
        return;
    }

    if (encounter)
    {
        stop(player);
        battleIntro = manageBattleIntro();
        battleIntro->start();
        preventInputs = true;
        return;
    }

    if (isWaterfallTile(player.x, player.y, player.l)
        || (player.l > 0 && isWaterfallTile(player.x, player.y, player.l - 1)))
    {
        player.surfing   = true;
        player.direction = player.previousDirection;
        if (player.direction == Entity::Direction::DOWN && isWaterTile(player.x, player.y + 1, player.l - 1))
            player.l--;
        move(player, true);
        preventInputs = true;
        return;
    }
    else if (!isWaterfallTile(player.x, player.y, player.l)
             && (isWaterfallTile(player.previousX, player.previousY, player.l)
                 || (player.l > 0 && isWaterfallTile(player.previousX, player.previousY, player.l - 1))))
    {
        stop(player);
    }

    if (preventInputs)
    {
        preventInputs = false;
        return;
    }

    if (inputs->B && !player.surfing)
        player.speed = Entity::Speed::RUN;
    else
        player.speed = Entity::Speed::WALK;

    if (inputs->A)
    {
        if (!player.surfing && isEntityFacingWaterTile(player))
        {
            player.surfing   = true;
            player.direction = player.previousDirection;
            move(player);
            return;
        }

        if (isEntityFacingTreeTile(player))
        {
            auto erasableEntity = dynamic_cast<ErasableEntity*>(facedEntity(player));
            if (erasableEntity)
                erasableEntity->startErase();
            return;
        }

        if (isEntityFacingRockTile(player))
        {
            auto erasableEntity = dynamic_cast<ErasableEntity*>(facedEntity(player));
            if (erasableEntity)
                erasableEntity->startErase();
            return;
        }

        if (isEntityFacingWaterfallTile(player))
        {
            player.surfing   = true;
            player.direction = player.previousDirection;
            if (player.direction == Entity::Direction::UP)
                player.l++;
            move(player, true);
            return;
        }

        if (auto entity = facedEntity(player))
        {
            if (entity->boulder)
            {
                entity->direction = player.previousDirection;
                move(*entity);
            }
            return;
        }
    }

    if (inputs->start)
    {
        stop(player);
        openMenu = true;
        return;
    }
    else
        openMenu = false;

    if (inputs->up)
    {
        player.previousDirection = Entity::Direction::UP;
        player.direction         = Entity::Direction::UP;
        move(player);
    }
    else if (inputs->down)
    {
        player.previousDirection = Entity::Direction::DOWN;
        player.direction         = Entity::Direction::DOWN;
        move(player);
    }
    else if (inputs->left)
    {
        player.previousDirection = Entity::Direction::LEFT;
        player.direction         = Entity::Direction::LEFT;
        move(player);
    }
    else if (inputs->right)
    {
        player.previousDirection = Entity::Direction::RIGHT;
        player.direction         = Entity::Direction::RIGHT;
        move(player);
    }
    else
    {
        stop(player);
    }
}

void MapScene::draw(Fps const* fps, RenderSizes rs)
{
    auto& player = Game::instance()->data.player;

    int dstTilePixelWidth  = TilePixelSize * rs.ww / rs.aw;
    int dstTilePixelHeight = TilePixelSize * rs.wh / rs.ah;

    SDL_Rect dstPlayerRect;
    dstPlayerRect.x = (rs.ww - dstTilePixelWidth) / 2;
    dstPlayerRect.y = (rs.wh - dstTilePixelHeight) / 2 - (EntityPixelHeight - TilePixelSize) * rs.wh / rs.ah;
    dstPlayerRect.w = dstTilePixelWidth + 1;
    dstPlayerRect.h = EntityPixelHeight * rs.wh / rs.ah + 1;

    int playerOffsetX = getPlayerOffsetX(fps, rs);
    int playerOffsetY = getPlayerOffsetY(fps, rs);

    for (size_t l = 0; l < map->getLevels().size(); ++l)
    {
        auto& level = map->getLevels()[l];

        for (size_t h = 0; h < level->getTileLayers().size(); ++h)
        {
            auto& layer = level->getTileLayers()[h];

            if (layer->getType() == TileLayer::Type::GROUND_LIGHTS || layer->getType() == TileLayer::Type::SOLID_LIGHTS
                || layer->getType() == TileLayer::Type::OVERLAY_LIGHTS)
            {
                if (!shouldShowNightTextures())
                    continue;
            }

            for (size_t j = 0; j < map->getNRow(); ++j)
            {
                for (size_t i = 0; i < map->getNCol(); ++i)
                {

                    SDL_Rect dstRect;
                    dstRect.x = i * dstTilePixelWidth + playerOffsetX;
                    dstRect.y = j * dstTilePixelHeight + playerOffsetY;
                    dstRect.w = dstTilePixelWidth;
                    dstRect.h = dstTilePixelHeight;

                    // Draw only visible tiles
                    if (!(dstRect.x >= -dstTilePixelWidth && dstRect.x <= (rs.ww + dstTilePixelWidth)
                          && dstRect.y >= -dstTilePixelHeight && dstRect.y <= rs.wh + dstTilePixelHeight))
                    {
                        continue;
                    }

                    std::string path;
                    SDL_Rect    srcRect;

                    auto& tile = (*layer.get())(i, j);
                    if (tile)
                    {
                        path = tile->getSpritePath();

                        if (layer->getType() == TileLayer::Type::GROUND_LIGHTS
                            || layer->getType() == TileLayer::Type::SOLID_LIGHTS
                            || layer->getType() == TileLayer::Type::OVERLAY_LIGHTS)
                        {
                            if (!lightsSprites.count(path))
                            {
                                lightsSprites[path].first = IMG_Load(path.c_str());
                                lightsSprites[path].second =
                                    SDL_CreateTextureFromSurface(renderer, lightsSprites[path].first);
                            }
                        }
                        else
                        {
                            if (!sprites.count(path))
                            {
                                sprites[path].first = IMG_Load(path.c_str());
                                sprites[path].second =
                                    RenderUtils::texture(renderer,
                                                         SDL_CreateTextureFromSurface(renderer, sprites[path].first),
                                                         shouldShowNightTextures());
                            }
                        }

                        auto& sprite = (layer->getType() == TileLayer::Type::GROUND_LIGHTS
                                        || layer->getType() == TileLayer::Type::SOLID_LIGHTS
                                        || layer->getType() == TileLayer::Type::OVERLAY_LIGHTS)
                                         ? lightsSprites[path]
                                         : sprites[path];

                        srcRect.x = tile->getCol() * TilePixelSize;
                        srcRect.y = tile->getRow() * TilePixelSize;
                        if (tile->isAnimated())
                        {
                            // 1 row animated autotiles
                            if (sprite.first->h == TilePixelSize)
                            {
                                int nSprites = sprite.first->w / TilePixelSize;
                                int addX     = (fps->next / 192 % nSprites) * TilePixelSize;
                                if (srcRect.x < sprite.first->w)
                                    srcRect.x += addX;
                            }

                            // 4 rows animated autotiles
                            if (sprite.first->h == 4 * TilePixelSize)
                            {
                                int nSprites = sprite.first->w / TilePixelSize;
                                int addX     = 3 * (fps->next / 128 % (nSprites / 3)) * TilePixelSize;
                                if (srcRect.x < sprite.first->w)
                                    srcRect.x += addX;
                            }
                        }
                        srcRect.w = TilePixelSize;
                        srcRect.h = TilePixelSize;
                    }

                    if (tile)
                    {
                        auto const& sprite = (layer->getType() == TileLayer::Type::GROUND_LIGHTS
                                              || layer->getType() == TileLayer::Type::SOLID_LIGHTS
                                              || layer->getType() == TileLayer::Type::OVERLAY_LIGHTS)
                                               ? lightsSprites[path]
                                               : sprites[path];
                        drawTile(fps, rs, tile, i, j, sprite, srcRect, dstRect);
                    }

                    if (layer->getType() == TileLayer::Type::SOLID_OVERLAY)
                    {
                        // Draw player
                        if (player.x == int(i) && player.y == int(j) && player.l == l)
                        {
                            drawPlayer(fps, rs, dstPlayerRect);
                        }

                        // Draw entities
                        for (auto it = entities.begin(); it != entities.end(); ++it)
                        {
                            auto const& entity = *(it->first.get());
                            auto&       sprite = *(it->second.get());
                            if (entity.l == l && entity.previousX == int(i) && entity.previousY == int(j))
                            {
                                int entityOffsetX = (entity.x - entity.previousX)
                                                  * (sprite.getAccumulatedTicks() + fps->tickPercentage())
                                                  / ((entity.speed + entity.previousSpeed) / 2.0) * dstTilePixelWidth;
                                int entityOffsetY = (entity.y - entity.previousY)
                                                  * (sprite.getAccumulatedTicks() + fps->tickPercentage())
                                                  / ((entity.speed + entity.previousSpeed) / 2.0) * dstTilePixelHeight;

                                SDL_Rect dstEntityRect;
                                dstEntityRect.x = i * dstTilePixelWidth + playerOffsetX + entityOffsetX;
                                dstEntityRect.y = j * dstTilePixelHeight + playerOffsetY + entityOffsetY
                                                - (EntityPixelHeight - TilePixelSize) * rs.wh / rs.ah;
                                dstEntityRect.w = dstTilePixelWidth + 1;
                                dstEntityRect.h = EntityPixelHeight * rs.wh / rs.ah + 1;
                                sprite.draw(entity, fps, rs, dstEntityRect);

                                tryDrawingHighGrass(fps, rs, entity, sprite, dstEntityRect);

                                if (isGrassTile(entity.previousX, entity.previousY, entity.l)
                                    && entity.speed == Entity::WALK)
                                    drawGrass(fps, rs, entity.previousX, entity.previousY, l);
                            }
                        }
                    }
                }

                if (layer->getType() == TileLayer::Type::SOLID_OVERLAY)
                {
                    for (size_t i = 0; i < map->getNCol(); ++i)
                    {
                        SDL_Rect dstRect;
                        dstRect.x = i * dstTilePixelWidth + playerOffsetX;
                        dstRect.y = j * dstTilePixelHeight + playerOffsetY;
                        dstRect.w = dstTilePixelWidth;
                        dstRect.h = dstTilePixelHeight;

                        // Draw only visible tiles
                        if (!(dstRect.x >= -dstTilePixelWidth && dstRect.x <= (rs.ww + dstTilePixelWidth)
                              && dstRect.y >= -dstTilePixelHeight && dstRect.y <= rs.wh + dstTilePixelHeight))
                        {
                            continue;
                        }

                        // Draw animated tiles
                        auto it = tilesAnimations.find({i, j});

                        if (it != tilesAnimations.end() && it->second && !it->second->isFinished())
                        {
                            if (isUnderWaterGrassTile(i, j, l))
                            {
                                dstRect.y -= dstTilePixelHeight;
                                dstRect.h += dstTilePixelHeight;
                            }

                            it->second->setDestinationRect(dstRect);
                            it->second->draw(fps, rs);
                        }
                    }
                }
            }
        }

        if (Game::instance()->isDebug())
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            auto& layer = level->getEventLayer();

            for (size_t i = 0; i < map->getNCol(); ++i)
            {
                for (size_t j = 0; j < map->getNRow(); ++j)
                {
                    auto& tile = (*layer.get())(i, j);
                    if (tile)
                    {
                        SDL_Rect dstRect;
                        dstRect.x = i * dstTilePixelWidth + playerOffsetX;
                        dstRect.y = j * dstTilePixelHeight + playerOffsetY;
                        dstRect.w = dstTilePixelWidth;
                        dstRect.h = dstTilePixelHeight;

                        // Draw only visible tiles
                        if (!(dstRect.x >= -dstTilePixelWidth && dstRect.x <= (rs.ww + dstTilePixelWidth)
                              && dstRect.y >= -dstTilePixelHeight && dstRect.y <= rs.wh + dstTilePixelHeight))
                        {
                            continue;
                        }

                        SDL_RenderDrawRect(renderer, &dstRect);
                    }
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
    }

    drawAmbientOverlay(fps, rs, playerOffsetX, playerOffsetY);

    drawWeather(fps, rs);

    if (!flash)
        drawFlashDarkness(fps, rs);

    if (battleIntro)
        battleIntro->draw(fps, rs);

    if (openMenu)
        menu->draw(fps, rs);

    if (fadeInAnimation->isRunning())
        fadeInAnimation->draw(fps, rs);

    if (fadeOutAnimation->isRunning())
        fadeOutAnimation->draw(fps, rs);
}

void MapScene::drawPlayer(Fps const* fps, RenderSizes rs, SDL_Rect dstPlayerRect)
{
    static constexpr int waterSpeed = 4;

    auto& player = Game::instance()->data.player;

    int dstTilePixelHeight = TilePixelSize * rs.wh / rs.ah;

    int waterMovementOffset =
        std::cos(std::numbers::pi
                 * (int(fps->accumulatedTicks) % waterSpeed - (waterSpeed - 1) / 2 + fps->tickPercentage() - 0.5)
                 / (waterSpeed / 2))
        * dstTilePixelHeight / 8;

    if (diving)
    {
        dstPlayerRect.y += waterMovementOffset;
        divingSprite->draw(player, fps, rs, dstPlayerRect);
        playerSurfSprite->draw(player, fps, rs, dstPlayerRect);
    }
    else if (player.surfing)
    {
        dstPlayerRect.y += waterMovementOffset;
        surfSprite->draw(player, fps, rs, dstPlayerRect);
        playerSurfSprite->draw(player, fps, rs, dstPlayerRect);
    }
    else if (stairsExitAnimation && stairsExitAnimation->isStarted())
    {
        stairsExitAnimation->setEntitySprite(&player, playerSprite.get());
        stairsExitAnimation->setDestinationRect(dstPlayerRect);
        stairsExitAnimation->draw(fps, rs);
    }
    else if (stairsEntranceAnimation && stairsEntranceAnimation->isRunning())
    {
        stairsEntranceAnimation->setEntitySprite(&player, playerSprite.get());
        stairsEntranceAnimation->setDestinationRect(dstPlayerRect);
        stairsEntranceAnimation->draw(fps, rs);
    }
    else if (ledgeAnimation && ledgeAnimation->isRunning())
    {
        ledgeAnimation->setEntitySprite(&player, playerSprite.get());
        ledgeAnimation->setDestinationRect(dstPlayerRect);
        ledgeAnimation->draw(fps, rs);
    }
    else
    {
        playerSprite->draw(player, fps, rs, dstPlayerRect);

        tryDrawingHighGrass(fps, rs, player, *playerSprite, dstPlayerRect);

        if (isGrassTile(player.previousX, player.previousY, player.l) && player.speed == Entity::WALK)
            drawGrass(fps, rs, player.previousX, player.previousY, player.l);
    }
}

void MapScene::drawTile(Fps const*                                   fps,
                        RenderSizes                                  rs,
                        Tile::TilePtr const&                         tile,
                        size_t                                       i,
                        size_t                                       j,
                        std::pair<SDL_Surface*, SDL_Texture*> const& sprite,
                        SDL_Rect                                     srcRect,
                        SDL_Rect                                     dstRect)
{

    SDL_RenderCopy(renderer, sprite.second, &srcRect, &dstRect);

    if (tile->isDoor())
    {
        if (doorClosingAnimation && doorClosingPosition == std::pair<int, int>{i, j})
        {
            doorClosingAnimation->setSprite(sprite);
            doorClosingAnimation->setSourceRect(srcRect);
            doorClosingAnimation->setDestinationRect(dstRect);
            doorClosingAnimation->draw(fps, rs);
        }

        if (doorOpeningAnimation && doorOpeningPosition == std::pair<int, int>{i, j})
        {
            doorOpeningAnimation->setSprite(sprite);
            doorOpeningAnimation->setSourceRect(srcRect);
            doorOpeningAnimation->setDestinationRect(dstRect);
            doorOpeningAnimation->draw(fps, rs);
        }
    }
}

void MapScene::drawAmbientOverlay(Fps const* /*fps*/, RenderSizes /*rs*/, size_t /*offsetX*/, size_t /*offsetY*/)
{
}

void MapScene::drawWeather(Fps const* fps, RenderSizes rs)
{
    if (weatherAnimation)
        weatherAnimation->draw(fps, rs);
}

void MapScene::drawFlashDarkness(Fps const* fps, RenderSizes rs)
{
    flashAnimation->draw(fps, rs);
}

void MapScene::initPlayerPosition(size_t x, size_t y, size_t l, Entity::Direction direction)
{
    auto& player                 = Game::instance()->data.player;
    player.x                     = x;
    player.y                     = y;
    player.previousX             = x;
    player.previousY             = y;
    player.l                     = l;
    player.direction             = Entity::Direction::NONE;
    playerSpriteInitialDirection = direction;

    if (isWaterTile(x, y, l))
        player.surfing = true;
}

void MapScene::initMovingPlayerPosition(size_t x, size_t y, size_t l, Entity::Direction direction)
{
    auto& player = Game::instance()->data.player;
    initPlayerPosition(x, y, l, direction);
    switch (direction)
    {
    case Entity::Direction::LEFT:
        player.previousX = player.x + 1;
        break;
    case Entity::Direction::RIGHT:
        player.previousX = player.x - 1;
        break;
    case Entity::Direction::UP:
        player.previousY = player.y + 1;
        break;
    case Entity::Direction::DOWN:
        player.previousY = player.y - 1;
        break;
    case Entity::Direction::NONE:
    default:
        break;
    }
    player.direction = direction;
}

void MapScene::initClosingDoor(size_t x, size_t y)
{
    doorClosingPosition  = {x, y};
    doorClosingAnimation = std::make_unique<DoorAnimation>(renderer, shouldShowNightTextures());
    doorClosingAnimation->setInverted(true);
}

void MapScene::initStairsEntrance(StairsAnimation::Direction direction)
{
    stairsEntranceAnimation = std::make_unique<StairsAnimation>(renderer, direction, shouldShowNightTextures());
    stairsEntranceAnimation->start();
}

void MapScene::startLedgeJump(Entity& entity)
{
    ledgeAnimation = std::make_unique<LedgeAnimation>(renderer, entity.direction, shouldShowNightTextures());
    ledgeAnimation->setStartingPosition(entity.x, entity.y);
    ledgeAnimation->start();
    incrementLedgeJump(entity);
}

void MapScene::incrementLedgeJump(Entity& entity)
{
    entity.direction = ledgeAnimation->getDirection();
    if (ledgeAnimation->getFinishPosition() != std::pair{entity.x, entity.y})
        move(entity, true);
    else
        stop(entity);
}

void MapScene::finishLedgeJump(Entity& entity)
{
    ledgeAnimation.release();
    stop(entity);
}

void MapScene::stop(Entity& entity)
{
    entity.direction = Entity::Direction::NONE;
    entity.previousY = entity.y;
    entity.previousX = entity.x;
}

void MapScene::move(Entity& entity, bool force)
{
    if (entity.direction == Entity::Direction::NONE)
        return;

    entity.previousX = entity.x;
    entity.previousY = entity.y;

    if (entity.direction == Entity::Direction::UP)
    {
        if (entity.y > 0 && canMove(entity, entity.x, entity.y - 1, entity.l, force))
        {
            if (!force && isLedgeTile(entity.x, entity.y - 1, entity.l)
                && isLedgePassable(entity, entity.x, entity.y - 1, entity.l))
            {
                startLedgeJump(entity);
            }
            else
            {
                entity.y--;
            }
        }
    }
    else if (entity.direction == Entity::Direction::DOWN)
    {
        if (entity.y < int(map->getNRow() - 1) && canMove(entity, entity.x, entity.y + 1, entity.l, force))
        {
            if (!force && isLedgeTile(entity.x, entity.y + 1, entity.l)
                && isLedgePassable(entity, entity.x, entity.y + 1, entity.l))
            {
                startLedgeJump(entity);
            }
            else
            {
                entity.y++;
            }
        }
    }
    else if (entity.direction == Entity::Direction::LEFT)
    {
        if (entity.x > 0 && canMove(entity, entity.x - 1, entity.y, entity.l, force))
        {
            if (!force && isLedgeTile(entity.x - 1, entity.y, entity.l)
                && isLedgePassable(entity, entity.x - 1, entity.y, entity.l))
            {
                startLedgeJump(entity);
            }
            else
            {
                entity.x--;
            }
        }
    }
    else if (entity.direction == Entity::Direction::RIGHT)
    {
        if (entity.x < int(map->getNCol() - 1) && canMove(entity, entity.x + 1, entity.y, entity.l, force))
        {
            if (!force && isLedgeTile(entity.x + 1, entity.y, entity.l)
                && isLedgePassable(entity, entity.x + 1, entity.y, entity.l))
            {
                startLedgeJump(entity);
            }
            else
            {
                entity.x++;
            }
        }
    }

    if (entity.x != entity.previousX || entity.y != entity.previousY)
    {
        if (isGrassTile(entity.x, entity.y, entity.l))
        {
            auto it = tilesAnimations.find({entity.x, entity.y});

            if (it == tilesAnimations.end())
            {
                tilesAnimations[{entity.x, entity.y}] =
                    std::make_unique<GrassAnimation>(renderer, shouldShowNightTextures());
                tilesAnimations[{entity.x, entity.y}]->start();
            }
            else
            {
                it->second->restart();
            }
        }

        if (isUnderWaterGrassTile(entity.x, entity.y, entity.l))
        {
            auto it = tilesAnimations.find({entity.x, entity.y});

            if (it == tilesAnimations.end())
            {
                tilesAnimations[{entity.x, entity.y}] =
                    std::make_unique<UnderWaterGrassAnimation>(renderer, shouldShowNightTextures());
                tilesAnimations[{entity.x, entity.y}]->start();
            }
            else
            {
                it->second->restart();
            }
        }

        if (entity.surfing && !isWaterTile(entity.x, entity.y, entity.l)
            && !isWaterfallTile(entity.x, entity.y, entity.l)
            && !(entity.l > 0 && isWaterfallTile(entity.x, entity.y, entity.l - 1)))
        {
            entity.surfing = false;
        }
    }
}

Entity* MapScene::entityAt(size_t x, size_t y, size_t l) const
{
    if (x >= map->getNCol())
        return nullptr;

    if (y >= map->getNRow())
        return nullptr;

    if (l >= map->getLevels().size())
        return nullptr;

    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
        auto entity = it->first.get();
        if (entity->x == int(x) && entity->y == int(y) && entity->l == l)
            return entity;
    }

    auto& player = Game::instance()->data.player;
    if (player.x == int(x) && player.y == int(y) && player.l == l)
        return &player;

    return {};
}

Entity* MapScene::entityPreviousAt(size_t x, size_t y, size_t l) const
{
    if (x >= map->getNCol())
        return nullptr;

    if (y >= map->getNRow())
        return nullptr;

    if (l >= map->getLevels().size())
        return nullptr;

    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
        auto entity = it->first.get();
        if (entity->previousX == int(x) && entity->previousY == int(y) && entity->l == l)
            return entity;
    }

    auto& player = Game::instance()->data.player;
    if (player.previousX == int(x) && player.previousY == int(y) && player.l == l)
        return &player;

    return {};
}

bool MapScene::canMove(Entity const& entity, size_t x, size_t y, size_t l, bool force) const
{
    if (force)
        return true;

    if (entityAt(x, y, l))
        return false;

    auto& level = map->getLevels()[l];

    auto& solidLayer = level->getTileLayers()[static_cast<size_t>(TileLayer::Type::SOLID)];
    auto& solidTile  = (*solidLayer.get())(x, y);
    if (solidTile)
        return false;

    auto& groundLayer = level->getTileLayers()[static_cast<size_t>(TileLayer::Type::GROUND)];
    auto& groundTile  = (*groundLayer.get())(x, y);
    if (!groundTile)
        return false;

    if (groundTile->isDoor())
        return false;

    if (isWaterTile(x, y, l) && !entity.surfing)
        return false;

    if (isWaterfallTile(x, y, l))
        return false;

    if (isTallGrassTile(x, y, l) && entity.speed != Entity::WALK)
        return false;

    if (isLedgeTile(x, y, l))
    {
        if (!isPlayerEntity(entity))
            return false;
        if (!isLedgePassable(entity, x, y, l))
            return false;
    }

    return true;
}

Entity* MapScene::facedEntity(Entity const& entity) const
{
    if (entity.previousDirection == Entity::Direction::UP)
    {
        if (auto fe = entityAt(entity.x, entity.y - 1, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::DOWN)
    {
        if (auto fe = entityAt(entity.x, entity.y + 1, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::LEFT)
    {
        if (auto fe = entityAt(entity.x - 1, entity.y, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::RIGHT)
    {
        if (auto fe = entityAt(entity.x + 1, entity.y, entity.l))
            return fe;
    }

    return nullptr;
}

bool MapScene::isEntityFacingTile(Entity const& entity, std::function<bool(size_t, size_t, size_t)> func) const
{
    if (entity.previousDirection == Entity::Direction::UP)
    {
        if (func(entity.x, entity.y - 1, entity.l))
            return true;
    }
    else if (entity.previousDirection == Entity::Direction::DOWN)
    {
        if (func(entity.x, entity.y + 1, entity.l))
            return true;
    }
    else if (entity.previousDirection == Entity::Direction::LEFT)
    {
        if (func(entity.x - 1, entity.y, entity.l))
            return true;
    }
    else if (entity.previousDirection == Entity::Direction::RIGHT)
    {
        if (func(entity.x + 1, entity.y, entity.l))
            return true;
    }

    return false;
}

bool MapScene::isNormalTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (!specialTile)
        return true;

    return false;
}

bool MapScene::isWaterTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == WATER)
            return true;
    }

    return false;
}

bool MapScene::isEntityFacingWaterTile(Entity const& entity) const
{
    return isEntityFacingTile(entity, [this](size_t x, size_t y, size_t l) {
        return this->isWaterTile(x, y, l);
    });
}

bool MapScene::isTreeTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == TREE)
            return true;
    }

    return false;
}

bool MapScene::isEntityFacingTreeTile(Entity const& entity) const
{
    return isEntityFacingTile(entity, [this](size_t x, size_t y, size_t l) {
        return this->isTreeTile(x, y, l);
    });
}

bool MapScene::isRockTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == ROCK)
            return true;
    }

    return false;
}

bool MapScene::isEntityFacingRockTile(Entity const& entity) const
{
    return isEntityFacingTile(entity, [this](size_t x, size_t y, size_t l) {
        return this->isRockTile(x, y, l);
    });
}

bool MapScene::isWaterfallTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == WATERFALL)
            return true;
    }

    return false;
}

bool MapScene::isEntityFacingWaterfallTile(Entity const& entity) const
{
    return isEntityFacingTile(entity,
                              [this](size_t x, size_t y, size_t l) {
                                  return this->isWaterfallTile(x, y, l);
                              })
        || isEntityFacingTile(entity, [this](size_t x, size_t y, size_t l) {
               if (l == 0)
                   return false;

               return this->isWaterfallTile(x, y, l - 1);
           });
}

bool MapScene::isGrassTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == GRASS)
            return true;
    }

    return false;
}

bool MapScene::isTallGrassTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == TALLGRASS)
            return true;
    }

    return false;
}

bool MapScene::isUnderWaterGrassTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == UNDERWATERGRASS)
            return true;
    }

    return false;
}

bool MapScene::isLedgeTile(size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == LEDGE)
            return true;
    }

    return false;
}

bool MapScene::isLedgePassable(Entity const& entity, size_t x, size_t y, size_t l) const
{
    auto& level = map->getLevels()[l];

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile && *(specialTile.get()) == LEDGE)
    {
        auto& eventLayer = level->getEventLayer();
        auto& event      = (*eventLayer.get())(x, y);
        if (event && event->getId() == Entity::getDirectionString(entity.direction))
        {
            return true;
        }
    }

    return false;
}

Event* MapScene::eventAt(size_t x, size_t y, size_t l) const
{
    if (x >= map->getNCol())
        return nullptr;

    if (y >= map->getNRow())
        return nullptr;

    if (l >= map->getLevels().size())
        return nullptr;

    auto& level = map->getLevels()[l];

    auto& eventLayer = level->getEventLayer();
    auto& event      = (*eventLayer.get())(x, y);
    return event.get();
}

Event* MapScene::facedEvent(Entity const& entity) const
{
    if (entity.previousDirection == Entity::Direction::UP)
    {
        if (auto fe = eventAt(entity.x, entity.y - 1, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::DOWN)
    {
        if (auto fe = eventAt(entity.x, entity.y + 1, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::LEFT)
    {
        if (auto fe = eventAt(entity.x - 1, entity.y, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::RIGHT)
    {
        if (auto fe = eventAt(entity.x + 1, entity.y, entity.l))
            return fe;
    }

    return nullptr;
}

Event* MapScene::facedPreviousEvent(Entity const& entity) const
{
    if (entity.previousDirection == Entity::Direction::UP)
    {
        if (auto fe = eventAt(entity.previousX, entity.previousY - 1, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::DOWN)
    {
        if (auto fe = eventAt(entity.previousX, entity.previousY + 1, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::LEFT)
    {
        if (auto fe = eventAt(entity.previousX - 1, entity.previousY, entity.l))
            return fe;
    }
    else if (entity.previousDirection == Entity::Direction::RIGHT)
    {
        if (auto fe = eventAt(entity.previousX + 1, entity.previousY, entity.l))
            return fe;
    }

    return nullptr;
}

bool MapScene::isPlayerEntity(Entity const* entity) const
{
    auto const& player = Game::instance()->data.player;
    return entity == &player;
}

bool MapScene::isPlayerEntity(Entity const& entity) const
{
    auto const& player = Game::instance()->data.player;
    return &entity == &player;
}

bool MapScene::manageEvents()
{
    return false;
}

bool MapScene::manageEncounters()
{
    auto const& player = Game::instance()->data.player;

    if (player.direction == Entity::Direction::NONE || encounteredPkmn)
        return false;

    // TODO all cases
    if (isGrassTile(player.x, player.y, player.l))
    {
        auto const& encounterMethods = map->getEncounterMethods();
        auto        it = std::find_if(encounterMethods.begin(), encounterMethods.end(), [=](EncounterMethod const& e) {
            return e.getType() == EncounterMethod::Type::LAND;
        });
        if (it != encounterMethods.end())
        {
            auto const& encounterMethod = *it;
            size_t      pkmnEncounter   = Utils::randuint(0, 99);

            if (Game::instance()->isDebug())
                std::cout << "PkmnEncounter: " << pkmnEncounter << "/" << encounterMethod.getDensity() << std::endl;

            if (pkmnEncounter < encounterMethod.getDensity())
            {
                std::random_device  rd;
                std::mt19937        gen(rd());
                auto const&         encounters = encounterMethod.getEncounters();
                std::vector<double> weights;
                double              accumulated = 0;
                for (auto const& e : encounters)
                {
                    accumulated += e.getPercentage();
                    weights.push_back(e.getPercentage());
                }
                for (auto& w : weights)
                {
                    w /= accumulated;
                }
                std::discrete_distribution<> d(weights.begin(), weights.end());
                int                          eIndex = d(gen);
                Encounter                    e      = encounters[eIndex];

                if (Game::instance()->isDebug())
                    std::cout << "PkmnDef: " << e.getPkmnId() << std::endl;

                PkmnDef::PkmnDefPtr pkmnDef = Game::instance()->data.pkmnDefFor(e.getPkmnId());
                if (pkmnDef)
                {
                    size_t level    = Utils::randuint(e.getLevelMin(), e.getLevelMax());
                    encounteredPkmn = std::make_shared<Pkmn>(pkmnDef, level);
                    encounteredPkmn->generateFromPkmnDef();
                    // TODO: temp
                    encounteredPkmn->setStatusCondition(Pkmn::StatusCondition::BADLY_POISON);
                }
                return true;
            }
        }
    }

    return false;
}

std::unique_ptr<BattleIntroAnimation> MapScene::manageBattleIntro()
{
    auto battleIntro = std::make_unique<BattleIntroAnimation>(renderer);
    battleIntro->setFile("resources/Graphics/Transitions/battle2.png");

    if (!encounteredPkmn)
    {
        // Force to implement logic for specific trainers
        battleIntro->stop();
    }

    return battleIntro;
}

bool MapScene::pushScene() const
{
    if (openPkmns && fadeOutAnimation->isFinished())
    {
        return true;
    }
    else if (openBag && fadeOutAnimation->isFinished())
    {
        return true;
    }
    else if (encounteredPkmn && battleIntro && battleIntro->isFinished())
    {
        return true;
    }

    return false;
}

void MapScene::popReset()
{
    fadeInAnimation->reset();
    fadeInAnimation->start();

    openPkmns = false;
    openBag   = false;
    encounteredPkmn.reset();
    battleIntro.reset();
}

std::unique_ptr<Scene> MapScene::nextScene()
{
    if (openPkmns && fadeOutAnimation->isFinished())
    {
        return std::make_unique<PkmnsScene>(renderer, emptyPkmnPtr);
    }
    else if (openBag && fadeOutAnimation->isFinished())
    {
        auto scene = std::make_unique<BagScene>(renderer);
        return scene;
    }
    else if (encounteredPkmn && battleIntro && battleIntro->isFinished())
    {
        auto scene = std::make_unique<EncounterScene>(renderer);
        scene->changeWeather(weather);
        scene->setEncounterPkmn(encounteredPkmn);
        auto& pkmns = Game::instance()->data.player.pkmns;
        for (size_t i = 0; i < pkmns.size(); ++i)
        {
            if (!pkmns[i]->isKO())
            {
                scene->setPlayerPkmn(pkmns[i]);
                break;
            }
        }
        return scene;
    }
    return nullptr;
}

std::tuple<size_t, size_t, size_t> MapScene::currentPlayerPosition() const
{
    auto const& player = Game::instance()->data.player;
    return {player.x, player.y, player.l};
}

void MapScene::changeWeather(Map::Weather weather)
{
    if (this->weather == weather)
        return;

    this->weather = weather;

    if (weather == Map::Weather::NONE)
    {
        weatherAnimation.release();
        return;
    }

    switch (weather)
    {
    // TODO: all weather animations
    default:
        weatherAnimation = std::make_unique<RainAnimation>(renderer, shouldShowNightTextures());
        break;
    }
}

bool MapScene::turnOnFlash()
{
    if (!flash)
    {
        if (!flashAnimation->isStarted())
        {
            flashAnimation->reset();
            flashAnimation->start();
            return true;
        }
    }

    return false;
}

size_t MapScene::getPlayerOffsetX(Fps const* fps, RenderSizes rs) const
{
    auto const& player = Game::instance()->data.player;

    int dstTilePixelWidth = TilePixelSize * rs.ww / rs.aw;

    int playerOffsetX = (rs.ww - dstTilePixelWidth) / 2
                      - (player.previousX
                         + (player.x - player.previousX) * (playerSprite->getAccumulatedTicks() + fps->tickPercentage())
                               / ((player.speed + player.previousSpeed) / 2.0))
                            * dstTilePixelWidth;

    return playerOffsetX;
}

size_t MapScene::getPlayerOffsetY(Fps const* fps, RenderSizes rs) const
{
    auto const& player = Game::instance()->data.player;

    int dstTilePixelHeight = TilePixelSize * rs.wh / rs.ah;

    int playerOffsetY = (rs.wh - dstTilePixelHeight) / 2
                      - (player.previousY
                         + (player.y - player.previousY) * (playerSprite->getAccumulatedTicks() + fps->tickPercentage())
                               / ((player.speed + player.previousSpeed) / 2.0))
                            * dstTilePixelHeight;

    return playerOffsetY;
}

void MapScene::drawGrass(Fps const* fps, RenderSizes rs, size_t x, size_t y, size_t l)
{
    int dstTilePixelWidth  = TilePixelSize * rs.ww / rs.aw;
    int dstTilePixelHeight = TilePixelSize * rs.wh / rs.ah;

    int playerOffsetX = getPlayerOffsetX(fps, rs);
    int playerOffsetY = getPlayerOffsetY(fps, rs);

    auto& level = map->getLevels()[l];

    auto groundLayerIt = std::find_if(level->getTileLayers().begin(), level->getTileLayers().end(), [=](auto const& e) {
        return e->getType() == TileLayer::Type::GROUND;
    });
    auto& groundTile   = (*groundLayerIt->get())(x, y);
    if (groundTile)
    {
        std::string path = groundTile->getSpritePath();

        SDL_Rect srcRect;
        srcRect.x = groundTile->getCol() * TilePixelSize;
        srcRect.y = groundTile->getRow() * TilePixelSize + TilePixelSize * 2 / 3;
        srcRect.w = TilePixelSize;
        srcRect.h = TilePixelSize - TilePixelSize * 2 / 3;

        SDL_Rect dstRect;
        dstRect.x = x * dstTilePixelWidth + playerOffsetX;
        dstRect.y = y * dstTilePixelHeight + playerOffsetY + dstTilePixelHeight * 2 / 3;
        dstRect.w = dstTilePixelWidth;
        dstRect.h = dstTilePixelHeight - dstTilePixelHeight * 2 / 3;

        SDL_RenderCopy(renderer, sprites[path].second, &srcRect, &dstRect);
    }
}

void MapScene::drawTallGrass(SDL_Rect dstRect,
                             size_t   x,
                             size_t   y,
                             size_t   l,
                             int      entityOffsetX,
                             int      entityOffsetY,
                             int      width,
                             int      height)
{
    auto& level = map->getLevels()[l];

    auto groundOverlayLayerIt =
        std::find_if(level->getTileLayers().begin(), level->getTileLayers().end(), [=](auto const& e) {
            return e->getType() == TileLayer::Type::GROUND_OVERLAY;
        });
    auto& groundOverlayTile = (*groundOverlayLayerIt->get())(x, y);
    if (groundOverlayTile)
    {
        std::string path = groundOverlayTile->getSpritePath();

        SDL_Rect srcRect;
        srcRect.x = groundOverlayTile->getCol() * TilePixelSize + entityOffsetX;
        srcRect.y = groundOverlayTile->getRow() * TilePixelSize + entityOffsetY;
        srcRect.w = width;
        srcRect.h = height;

        SDL_RenderCopy(renderer, sprites[path].second, &srcRect, &dstRect);
        return;
    }

    auto groundLayerIt = std::find_if(level->getTileLayers().begin(), level->getTileLayers().end(), [=](auto const& e) {
        return e->getType() == TileLayer::Type::GROUND;
    });
    auto& groundTile   = (*groundLayerIt->get())(x, y);
    if (groundTile)
    {
        std::string path = groundTile->getSpritePath();

        SDL_Rect srcRect;
        srcRect.x = groundTile->getCol() * TilePixelSize + entityOffsetX;
        srcRect.y = groundTile->getRow() * TilePixelSize + entityOffsetY;
        srcRect.w = width;
        srcRect.h = height;

        SDL_RenderCopy(renderer, sprites[path].second, &srcRect, &dstRect);
        return;
    }
}

void MapScene::tryDrawingHighGrass(Fps const*    fps,
                                   RenderSizes   rs,
                                   Entity const& entity,
                                   Sprite const& sprite,
                                   SDL_Rect      dstRect)
{
    int dstTilePixelWidth  = TilePixelSize * rs.ww / rs.aw;
    int dstTilePixelHeight = TilePixelSize * rs.wh / rs.ah;

    dstRect.y += (EntityPixelHeight - TilePixelSize) * rs.wh / rs.ah;
    dstRect.w = dstTilePixelWidth;
    dstRect.h = dstTilePixelHeight;

    if (entity.x == entity.previousX && entity.y == entity.previousY)
    {
        if (isTallGrassTile(entity.x, entity.y, entity.l))
            drawTallGrass(dstRect, entity.x, entity.y, entity.l, 0, 0, TilePixelSize, TilePixelSize);

        return;
    }

    float entityOffset =
        (sprite.getAccumulatedTicks() + fps->tickPercentage()) / ((entity.speed + entity.previousSpeed) / 2.0);

    if (entity.y != entity.previousY && entity.direction == Entity::Direction::UP)
    {
        if (isTallGrassTile(entity.x, entity.y, entity.l))
        {
            SDL_Rect dstRect1 = dstRect;
            dstRect1.h        = entityOffset * dstTilePixelHeight;
            drawTallGrass(dstRect1,
                          entity.x,
                          entity.y,
                          entity.l,
                          0,
                          (1.0 - entityOffset) * TilePixelSize,
                          TilePixelSize,
                          entityOffset * TilePixelSize);

            SDL_Rect dstRect2 = dstRect;
            dstRect2.y += entityOffset * dstTilePixelHeight;
            dstRect2.h = (1.0 - entityOffset) * dstTilePixelHeight;
            drawTallGrass(dstRect2,
                          entity.x,
                          entity.y + 1,
                          entity.l,
                          0,
                          0,
                          TilePixelSize,
                          (1.0 - entityOffset) * TilePixelSize);
        }
        else if (!isTallGrassTile(entity.x, entity.y, entity.l) && isTallGrassTile(entity.x, entity.y + 1, entity.l))
        {
            SDL_Rect dstRect1 = dstRect;
            dstRect1.y += entityOffset * dstTilePixelHeight;
            dstRect1.h = (1.0 - entityOffset) * dstTilePixelHeight;
            drawTallGrass(dstRect1,
                          entity.x,
                          entity.y + 1,
                          entity.l,
                          0,
                          0,
                          TilePixelSize,
                          (1.0 - entityOffset) * TilePixelSize);
        }
        return;
    }

    if (entity.y != entity.previousY && entity.direction == Entity::Direction::DOWN)
    {
        if (isTallGrassTile(entity.x, entity.y, entity.l))
        {
            SDL_Rect dstRect1 = dstRect;
            dstRect1.y += (1.0 - entityOffset) * dstTilePixelHeight;
            dstRect1.h = entityOffset * dstTilePixelHeight;
            drawTallGrass(dstRect1, entity.x, entity.y, entity.l, 0, 0, TilePixelSize, entityOffset * TilePixelSize);

            if (isTallGrassTile(entity.x, entity.y - 1, entity.l))
            {
                SDL_Rect dstRect2 = dstRect;
                dstRect2.h        = (1.0 - entityOffset) * dstTilePixelHeight;
                drawTallGrass(dstRect2,
                              entity.x,
                              entity.y - 1,
                              entity.l,
                              0,
                              entityOffset * TilePixelSize,
                              TilePixelSize,
                              (1.0 - entityOffset) * TilePixelSize);
            }
        }
        else if (!isTallGrassTile(entity.x, entity.y, entity.l) && isTallGrassTile(entity.x, entity.y - 1, entity.l))
        {
            SDL_Rect dstRect1 = dstRect;
            dstRect1.y += (1.0 - entityOffset) * dstTilePixelHeight;
            dstRect1.h = entityOffset * dstTilePixelHeight;
            drawTallGrass(dstRect1, entity.x, entity.y, entity.l, 0, 0, TilePixelSize, entityOffset * TilePixelSize);

            SDL_Rect dstRect2 = dstRect;
            dstRect2.h        = (1.0 - entityOffset) * dstTilePixelHeight;
            drawTallGrass(dstRect2,
                          entity.x,
                          entity.y - 1,
                          entity.l,
                          0,
                          entityOffset * TilePixelSize,
                          TilePixelSize,
                          (1.0 - entityOffset) * TilePixelSize);
        }
        return;
    }

    if (entity.x != entity.previousX && entity.direction == Entity::Direction::RIGHT)
    {
        if (isTallGrassTile(entity.x, entity.y, entity.l))
        {
            if (isTallGrassTile(entity.x - 1, entity.y, entity.l))
            {
                SDL_Rect dstRect1 = dstRect;
                dstRect1.w        = (1.0 - entityOffset) * dstTilePixelWidth;
                drawTallGrass(dstRect1,
                              entity.x - 1,
                              entity.y,
                              entity.l,
                              entityOffset * TilePixelSize,
                              0,
                              (1.0 - entityOffset) * TilePixelSize,
                              TilePixelSize);
            }

            SDL_Rect dstRect2 = dstRect;
            dstRect2.x += (1.0 - entityOffset) * dstTilePixelWidth;
            dstRect2.w = entityOffset * dstTilePixelWidth;
            drawTallGrass(dstRect2, entity.x, entity.y, entity.l, 0, 0, entityOffset * TilePixelSize, TilePixelSize);
        }
        else if (!isTallGrassTile(entity.x, entity.y, entity.l) && isTallGrassTile(entity.x - 1, entity.y, entity.l))
        {
            SDL_Rect dstRect1 = dstRect;
            dstRect1.w        = (1.0 - entityOffset) * dstTilePixelWidth;
            drawTallGrass(dstRect1,
                          entity.x - 1,
                          entity.y,
                          entity.l,
                          entityOffset * TilePixelSize,
                          0,
                          (1.0 - entityOffset) * TilePixelSize,
                          TilePixelSize);
        }
        return;
    }

    if (entity.x != entity.previousX && entity.direction == Entity::Direction::LEFT)
    {
        if (isTallGrassTile(entity.x, entity.y, entity.l))
        {
            if (isTallGrassTile(entity.x + 1, entity.y, entity.l))
            {
                SDL_Rect dstRect1 = dstRect;
                dstRect1.x += entityOffset * dstTilePixelWidth;
                dstRect1.w = (1.0 - entityOffset) * dstTilePixelWidth;
                drawTallGrass(dstRect1,
                              entity.x + 1,
                              entity.y,
                              entity.l,
                              0,
                              0,
                              (1.0 - entityOffset) * TilePixelSize,
                              TilePixelSize);
            }

            SDL_Rect dstRect2 = dstRect;
            dstRect2.w        = entityOffset * dstTilePixelWidth;
            drawTallGrass(dstRect2,
                          entity.x,
                          entity.y,
                          entity.l,
                          (1.0 - entityOffset) * TilePixelSize,
                          0,
                          entityOffset * TilePixelSize,
                          TilePixelSize);
        }
        else if (!isTallGrassTile(entity.x, entity.y, entity.l) && isTallGrassTile(entity.x + 1, entity.y, entity.l))
        {
            SDL_Rect dstRect1 = dstRect;
            dstRect1.x += entityOffset * dstTilePixelWidth;
            dstRect1.w = (1.0 - entityOffset) * dstTilePixelWidth;
            drawTallGrass(dstRect1,
                          entity.x + 1,
                          entity.y,
                          entity.l,
                          0,
                          0,
                          (1.0 - entityOffset) * TilePixelSize,
                          TilePixelSize);
        }
        return;
    }
}
