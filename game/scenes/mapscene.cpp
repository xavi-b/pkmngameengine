#include "mapscene.h"

#include "animations/map/grassanimation.h"
#include "animations/weather/rainanimation.h"
#include "bagscene.h"
#include "game.h"
#include "pkmnsscene.h"

#include <fstream>
#include <iostream>
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
    playerSprite = std::make_unique<Sprite>(renderer);
    if (Game::instance()->data.player.getGender() == Player::Gender::BOY)
        playerSprite->load("resources/Graphics/Characters/boy_run.png", shouldShowNightTextures());
    else
        playerSprite->load("resources/Graphics/Characters/girl_run.png", shouldShowNightTextures());
    playerSprite->forceSpriteDirection(playerSpriteInitialDirection);

    fadeOutAnimation = std::make_unique<FadeAnimation>(renderer, false);
    fadeInAnimation  = std::make_unique<FadeAnimation>(renderer, true);
    fadeInAnimation->reset();
    fadeInAnimation->start();
}

void MapScene::update(Inputs const* inputs)
{
    auto& player = Game::instance()->data.player;

    player.previousSpeed = player.speed;

    playerSprite->setAccumulatedTicks((playerSprite->getAccumulatedTicks() + 1) % player.speed);

    if (player.direction == Entity::Direction::NONE && player.speed == Entity::Speed::WALK)
        playerSprite->setAccumulatedTicks(0);

    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
        auto& entity = *(it->first.get());
        auto& sprite = *(it->second.get());

        sprite.setAccumulatedTicks((sprite.getAccumulatedTicks() + 1) % entity.speed);
    }

    if (weatherAnimation)
    {
        if (!weatherAnimation->isStarted())
            weatherAnimation->start();

        weatherAnimation->incrementTicks();
    }

    if (fadeInAnimation->isStarted())
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

    if (fadeOutAnimation->isStarted() && !fadeOutAnimation->isFinished())
    {
        fadeOutAnimation->incrementTicks();
        return;
    }

    if (doorOpeningAnimation && doorOpeningAnimation->isStarted() && !doorOpeningAnimation->isFinished())
    {
        doorOpeningAnimation->incrementTicks();
        return;
    }

    if (doorClosingAnimation && doorClosingAnimation->isStarted() && !doorClosingAnimation->isFinished())
    {
        doorClosingAnimation->incrementTicks();
    }

    if (battleIntro && !battleIntro->isFinished())
    {
        battleIntro->incrementTicks();
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
        return;
    }

    if (encounter)
    {
        stop(player);
        battleIntro = manageBattleIntro();
        battleIntro->start();
        return;
    }

    if (inputs->B)
        player.speed = Entity::Speed::RUN;
    else
        player.speed = Entity::Speed::WALK;

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
        player.direction = Entity::Direction::UP;
        move(player);
    }
    else if (inputs->down)
    {
        player.direction = Entity::Direction::DOWN;
        move(player);
    }
    else if (inputs->left)
    {
        player.direction = Entity::Direction::LEFT;
        move(player);
    }
    else if (inputs->right)
    {
        player.direction = Entity::Direction::RIGHT;
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

    int playerOffsetX = (rs.ww - dstTilePixelWidth) / 2
                      - (player.previousX
                         + (player.x - player.previousX) * (playerSprite->getAccumulatedTicks() + fps->tickPercentage())
                               / ((player.speed + player.previousSpeed) / 2.0))
                            * dstTilePixelWidth;
    int playerOffsetY = (rs.wh - dstTilePixelHeight) / 2
                      - (player.previousY
                         + (player.y - player.previousY) * (playerSprite->getAccumulatedTicks() + fps->tickPercentage())
                               / ((player.speed + player.previousSpeed) / 2.0))
                            * dstTilePixelHeight;

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

                    SDL_Rect dstRect;
                    dstRect.x = i * dstTilePixelWidth + playerOffsetX;
                    dstRect.y = j * dstTilePixelHeight + playerOffsetY;
                    dstRect.w = dstTilePixelWidth;
                    dstRect.h = dstTilePixelHeight;

                    // Draw only visible tiles
                    if (dstRect.x >= -dstTilePixelWidth && dstRect.x <= (rs.ww + dstTilePixelWidth)
                        && dstRect.y >= -dstTilePixelHeight && dstRect.y <= rs.wh + dstTilePixelHeight)
                    {
                        if (tile)
                        {
                            auto& sprite = (layer->getType() == TileLayer::Type::GROUND_LIGHTS
                                            || layer->getType() == TileLayer::Type::SOLID_LIGHTS
                                            || layer->getType() == TileLayer::Type::OVERLAY_LIGHTS)
                                             ? lightsSprites[path]
                                             : sprites[path];

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

                        if (layer->getType() == TileLayer::Type::SOLID_OVERLAY)
                        {
                            // Draw player
                            if (player.x == int(i) && player.y == int(j) && player.l == l)
                            {
                                playerSprite->draw(player, fps, &dstPlayerRect);
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
                                                      / ((entity.speed + entity.previousSpeed) / 2.0)
                                                      * dstTilePixelWidth;
                                    int entityOffsetY = (entity.y - entity.previousY)
                                                      * (sprite.getAccumulatedTicks() + fps->tickPercentage())
                                                      / ((entity.speed + entity.previousSpeed) / 2.0)
                                                      * dstTilePixelHeight;

                                    SDL_Rect dstEntityRect;
                                    dstEntityRect.x = i * dstTilePixelWidth + playerOffsetX + entityOffsetX;
                                    dstEntityRect.y = j * dstTilePixelHeight + playerOffsetY + entityOffsetY
                                                    - (EntityPixelHeight - TilePixelSize) * rs.wh / rs.ah;
                                    dstEntityRect.w = dstTilePixelWidth + 1;
                                    dstEntityRect.h = EntityPixelHeight * rs.wh / rs.ah + 1;
                                    sprite.draw(entity, fps, &dstEntityRect);
                                }
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
                        if (dstRect.x >= -dstTilePixelWidth && dstRect.x <= (rs.ww + dstTilePixelWidth)
                            && dstRect.y >= -dstTilePixelHeight && dstRect.y <= rs.wh + dstTilePixelHeight)
                        {
                            // Draw grass animated tiles
                            auto& specialTileLayer = level->getSpecialTileLayer();
                            auto& specialTile      = (*specialTileLayer.get())(i, j);
                            if (specialTile && *(specialTile.get()) == GRASS)
                            {
                                auto entity = entityPreviousAt(i, j, l);
                                if (entity && entity->speed == Entity::WALK)
                                {
                                    auto groundLayerIt =
                                        std::find_if(level->getTileLayers().begin(),
                                                     level->getTileLayers().end(),
                                                     [=](auto const& e) {
                                                         return e->getType() == TileLayer::Type::GROUND;
                                                     });
                                    auto& groundTile = (*groundLayerIt->get())(i, j);
                                    if (groundTile)
                                    {
                                        std::string path = groundTile->getSpritePath();

                                        SDL_Rect srcRect;
                                        srcRect.x = groundTile->getCol() * TilePixelSize;
                                        srcRect.y = groundTile->getRow() * TilePixelSize + TilePixelSize * 2 / 3;
                                        srcRect.w = TilePixelSize;
                                        srcRect.h = TilePixelSize - TilePixelSize * 2 / 3;

                                        SDL_Rect dstRect;
                                        dstRect.x = i * dstTilePixelWidth + playerOffsetX;
                                        dstRect.y = j * dstTilePixelHeight + playerOffsetY + dstTilePixelHeight * 2 / 3;
                                        dstRect.w = dstTilePixelWidth;
                                        dstRect.h = dstTilePixelHeight - dstTilePixelHeight * 2 / 3;

                                        auto& sprite = (layer->getType() == TileLayer::Type::GROUND_LIGHTS
                                                        || layer->getType() == TileLayer::Type::SOLID_LIGHTS
                                                        || layer->getType() == TileLayer::Type::OVERLAY_LIGHTS)
                                                         ? lightsSprites[path]
                                                         : sprites[path];

                                        SDL_RenderCopy(renderer, sprite.second, &srcRect, &dstRect);
                                    }
                                }

                                auto it = tilesAnimations.find({i, j});

                                if (it != tilesAnimations.end() && it->second && !it->second->isFinished())
                                {
                                    it->second->setDestinationRect(dstRect);
                                    it->second->draw(fps, rs);
                                }
                            }
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
                        if (dstRect.x >= -dstTilePixelWidth && dstRect.x <= (rs.ww + dstTilePixelWidth)
                            && dstRect.y >= -dstTilePixelHeight && dstRect.y <= rs.wh + dstTilePixelHeight)
                        {
                            SDL_RenderDrawRect(renderer, &dstRect);
                        }
                    }
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
    }

    drawAmbientOverlay(fps, rs, playerOffsetX, playerOffsetY);

    drawWeather(fps, rs);

    if (battleIntro)
        battleIntro->draw(fps, rs);

    if (openMenu)
        menu->draw(fps, rs);

    if (fadeInAnimation->isStarted() && !fadeInAnimation->isFinished())
        fadeInAnimation->draw(fps, rs);

    if (fadeOutAnimation->isStarted() && !fadeOutAnimation->isFinished())
        fadeOutAnimation->draw(fps, rs);
}

void MapScene::drawAmbientOverlay(Fps const* /*fps*/, RenderSizes /*rs*/, size_t /*offsetX*/, size_t /*offsetY*/)
{
}

void MapScene::drawWeather(Fps const* fps, RenderSizes rs)
{
    if (weatherAnimation)
        weatherAnimation->draw(fps, rs);
}

void MapScene::initPlayerPosition(int x, int y, Entity::Direction direction)
{
    auto& player                 = Game::instance()->data.player;
    player.x                     = x;
    player.y                     = y;
    player.previousX             = x;
    player.previousY             = y;
    player.direction             = Entity::Direction::NONE;
    playerSpriteInitialDirection = direction;
}

void MapScene::initMovingPlayerPosition(int x, int y, Entity::Direction direction)
{
    auto& player = Game::instance()->data.player;
    initPlayerPosition(x, y, direction);
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

void MapScene::initClosingDoor(int x, int y)
{
    doorClosingPosition  = {x, y};
    doorClosingAnimation = std::make_unique<DoorAnimation>(renderer, shouldShowNightTextures());
    doorClosingAnimation->setInverted(true);
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
        if (entity.y > 0 && canMove(entity.x, entity.y - 1, entity.l, force))
            entity.y--;
    }
    else if (entity.direction == Entity::Direction::DOWN)
    {
        if (entity.y < int(map->getNRow() - 1) && canMove(entity.x, entity.y + 1, entity.l, force))
            entity.y++;
    }
    else if (entity.direction == Entity::Direction::LEFT)
    {
        if (entity.x > 0 && canMove(entity.x - 1, entity.y, entity.l, force))
            entity.x--;
    }
    else if (entity.direction == Entity::Direction::RIGHT)
    {
        if (entity.x < int(map->getNCol() - 1) && canMove(entity.x + 1, entity.y, entity.l, force))
            entity.x++;
    }

    if (entity.x != entity.previousX || entity.y != entity.previousY)
    {
        auto& level            = map->getLevels()[entity.l];
        auto& specialTileLayer = level->getSpecialTileLayer();
        auto& specialTile      = (*specialTileLayer.get())(entity.x, entity.y);
        if (specialTile && *(specialTile.get()) == GRASS)
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
    }
}

Entity* MapScene::entityAt(size_t x, size_t y, size_t l) const
{
    if (x > map->getNCol())
        return nullptr;

    if (y > map->getNRow())
        return nullptr;

    if (l > map->getLevels().size())
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
    if (x > map->getNCol())
        return nullptr;

    if (y > map->getNRow())
        return nullptr;

    if (l > map->getLevels().size())
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

bool MapScene::canMove(size_t x, size_t y, size_t l, bool force) const
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

    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(x, y);
    if (specialTile)
    {
        if (*(specialTile.get()) == WATER && !surfing)
            return false;
    }

    return true;
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

    auto& level            = map->getLevels()[player.l];
    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile      = (*specialTileLayer.get())(player.x, player.y);
    if (!specialTile)
        return false;

    if (*(specialTile.get()) == GRASS)
    {
        // TODO all cases
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

std::pair<size_t, size_t> MapScene::currentPlayerPosition() const
{
    return {Game::instance()->data.player.x, Game::instance()->data.player.y};
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
