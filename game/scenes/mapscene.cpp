#include "mapscene.h"

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

    playerSprite = std::make_unique<Sprite>(renderer);
    if (Game::instance()->data.player.getGender() == Player::Gender::BOY)
        playerSprite->load("resources/Graphics/Characters/boy_run.png");
    else
        playerSprite->load("resources/Graphics/Characters/girl_run.png");

    menu = std::make_unique<Menu>(renderer);
    menu->init();

    fadeOutAnimation = std::make_unique<FadeAnimation>(renderer, false);
    fadeInAnimation  = std::make_unique<FadeAnimation>(renderer, true);
    fadeInAnimation->reset();
    fadeInAnimation->start();
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

void MapScene::update(Inputs const* inputs)
{
    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
        auto& entity = *(it->first.get());
        auto& sprite = *(it->second.get());

        sprite.setAccumulatedTicks((sprite.getAccumulatedTicks() + 1) % entity.speed);
    }

    if (fadeInAnimation->isStarted())
    {
        fadeInAnimation->incrementTicks();
    }

    if (fadeOutAnimation->isStarted() && !fadeOutAnimation->isFinished())
    {
        fadeOutAnimation->incrementTicks();
        return;
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

    if (weatherAnimation)
    {
        if (!weatherAnimation->isStarted())
            weatherAnimation->start();

        weatherAnimation->incrementTicks();
    }

    auto& player = Game::instance()->data.player;

    player.previousSpeed = player.speed;

    playerSprite->setAccumulatedTicks((playerSprite->getAccumulatedTicks() + 1) % player.speed);

    if (player.direction == Entity::Direction::NONE && player.speed == Entity::Speed::WALK)
        playerSprite->setAccumulatedTicks(0);

    if (playerSprite->getAccumulatedTicks() != 0)
        return;

    bool event     = manageEvents();
    bool encounter = manageEncounters();

    player.direction = Entity::Direction::NONE;
    player.previousY = player.y;
    player.previousX = player.x;

    if (event)
    {
        return;
    }

    if (encounter)
    {
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
                        if (!sprites.count(path))
                        {
                            sprites[path].first  = IMG_Load(path.c_str());
                            sprites[path].second = SDL_CreateTextureFromSurface(renderer, sprites[path].first);
                        }

                        srcRect.x = tile->getCol() * TilePixelSize;
                        srcRect.y = tile->getRow() * TilePixelSize;
                        srcRect.w = TilePixelSize;
                        srcRect.h = TilePixelSize;
                    }

                    SDL_Rect dstRect;
                    dstRect.x = i * dstTilePixelWidth + playerOffsetX;
                    dstRect.y = j * dstTilePixelHeight + playerOffsetY;
                    dstRect.w = dstTilePixelWidth + 1;
                    dstRect.h = dstTilePixelHeight + 1;

                    // draw only visible tiles
                    if (dstRect.x >= -dstTilePixelWidth && dstRect.x <= (rs.ww + dstTilePixelWidth)
                        && dstRect.y >= -dstTilePixelHeight && dstRect.y <= rs.wh + dstTilePixelHeight)
                    {
                        if (tile)
                            SDL_RenderCopy(renderer, sprites[path].second, &srcRect, &dstRect);

                        if (player.x == int(i) && player.y == int(j) && player.l == l
                            && layer->getType() == TileLayer::Type::SOLID)
                        {
                            playerSprite->draw(player, fps, &dstPlayerRect);
                        }

                        for (auto it = entities.begin(); it != entities.end(); ++it)
                        {
                            auto const& entity = *(it->first.get());
                            auto&       sprite = *(it->second.get());
                            if (entity.l == l && entity.previousX == int(i) && entity.previousY == int(j)
                                && layer->getType() == TileLayer::Type::SOLID)
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
                                sprite.draw(entity, fps, &dstEntityRect);
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
                        dstRect.w = dstTilePixelWidth + 1;
                        dstRect.h = dstTilePixelHeight + 1;

                        // draw only visible tiles
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

    drawWeather(fps, rs);

    if (Game::instance()->isNight())
        drawNight();

    if (battleIntro)
        battleIntro->draw(fps, rs);

    if (openMenu)
        menu->draw(fps, rs);

    if (fadeInAnimation->isStarted() && !fadeInAnimation->isFinished())
        fadeInAnimation->draw(fps, rs);

    if (fadeOutAnimation->isStarted() && !fadeOutAnimation->isFinished())
        fadeOutAnimation->draw(fps, rs);
}

void MapScene::drawWeather(Fps const* fps, RenderSizes rs)
{
    if (weatherAnimation)
        weatherAnimation->draw(fps, rs);
}

void MapScene::drawNight()
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,
                           TileLayer::NightColor[0],
                           TileLayer::NightColor[1],
                           TileLayer::NightColor[2],
                           TileLayer::NightColor[3]);

    SDL_RenderFillRect(renderer, NULL);
}

void MapScene::initPlayerPosition(int x, int y, Entity::Direction direction)
{
    Game::instance()->data.player.x         = x;
    Game::instance()->data.player.y         = y;
    Game::instance()->data.player.previousX = x;
    Game::instance()->data.player.previousY = y;
    Game::instance()->data.player.direction = Entity::Direction::NONE;
    playerSprite->forceSpriteDirection(direction);
}

void MapScene::move(Entity& entity)
{
    if (entity.direction == Entity::Direction::UP)
    {
        for (size_t l = 0; l < map->getLevels().size(); ++l)
        {
            auto& level = map->getLevels()[l];

            if (l != entity.l)
                continue;

            for (size_t h = 0; h < level->getTileLayers().size(); ++h)
            {
                auto& layer = level->getTileLayers()[h];

                if (layer->getType() != TileLayer::Type::SOLID)
                    continue;

                if (entity.y > 0)
                {
                    auto& tile = (*layer.get())(entity.x, entity.y - 1);
                    if (!tile && !entityAt(entity.x, entity.y - 1, l))
                    {
                        entity.y--;
                    }
                }
            }
        }
    }
    else if (entity.direction == Entity::Direction::DOWN)
    {
        for (size_t l = 0; l < map->getLevels().size(); ++l)
        {
            auto& level = map->getLevels()[l];

            if (l != entity.l)
                continue;

            for (size_t h = 0; h < level->getTileLayers().size(); ++h)
            {
                auto& layer = level->getTileLayers()[h];

                if (layer->getType() != TileLayer::Type::SOLID)
                    continue;

                if (entity.y < int(map->getNRow() - 1))
                {
                    auto& tile = (*layer.get())(entity.x, entity.y + 1);
                    if (!tile && !entityAt(entity.x, entity.y + 1, l))
                    {
                        entity.y++;
                    }
                }
            }
        }
    }
    else if (entity.direction == Entity::Direction::LEFT)
    {
        for (size_t l = 0; l < map->getLevels().size(); ++l)
        {
            auto& level = map->getLevels()[l];

            if (l != entity.l)
                continue;

            for (size_t h = 0; h < level->getTileLayers().size(); ++h)
            {
                auto& layer = level->getTileLayers()[h];

                if (layer->getType() != TileLayer::Type::SOLID)
                    continue;

                if (entity.x > 0)
                {
                    auto& tile = (*layer.get())(entity.x - 1, entity.y);
                    if (!tile && !entityAt(entity.x - 1, entity.y, l))
                    {
                        entity.x--;
                    }
                }
            }
        }
    }
    else if (entity.direction == Entity::Direction::RIGHT)
    {
        for (size_t l = 0; l < map->getLevels().size(); ++l)
        {
            auto& level = map->getLevels()[l];

            if (l != entity.l)
                continue;

            for (size_t h = 0; h < level->getTileLayers().size(); ++h)
            {
                auto& layer = level->getTileLayers()[h];

                if (layer->getType() != TileLayer::Type::SOLID)
                    continue;

                if (entity.x < int(map->getNCol() - 1))
                {
                    auto& tile = (*layer.get())(entity.x + 1, entity.y);
                    if (!tile && !entityAt(entity.x + 1, entity.y, l))
                    {
                        entity.x++;
                    }
                }
            }
        }
    }
}

Entity* MapScene::entityAt(size_t x, size_t y, size_t l)
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

bool MapScene::manageEvents()
{
    return false;
}

bool MapScene::manageEncounters()
{
    if (Game::instance()->data.player.direction == Entity::Direction::NONE || encounteredPkmn)
        return false;

    auto& level            = map->getLevels()[Game::instance()->data.player.l];
    auto& specialTileLayer = level->getSpecialTileLayer();
    auto& specialTile = (*specialTileLayer.get())(Game::instance()->data.player.x, Game::instance()->data.player.y);
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

bool MapScene::entitiesShouldFreeze() const
{
    return openMenu || battleIntro;
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
        weatherAnimation = std::make_unique<RainAnimation>(renderer);
        break;
    }
}
