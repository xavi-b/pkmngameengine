#include "mapscene.h"

#include "game.h"

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

    if (Game::instance()->data.player.gender == 0)
        playerSurface = IMG_Load("resources/Graphics/Characters/boy_run.png");
    else
        playerSurface = IMG_Load("resources/Graphics/Characters/girl_run.png");

    playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
}

MapScene::~MapScene()
{
    for (auto const& [key, value] : sprites)
    {
        auto [surface, texture] = value;

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    SDL_DestroyTexture(playerTexture);
    SDL_FreeSurface(playerSurface);
}

void MapScene::update(Inputs const* inputs)
{
    previousSpeed = speed;

    speed = WALK;
    if (inputs->B)
        speed = RUN;

    accumulatedTicks = (accumulatedTicks + 1) % speed;

    if (direction == NONE && speed == WALK)
        accumulatedTicks = 0;

    if (accumulatedTicks != 0)
        return;

    bool encounter = manageEncounters();

    direction       = NONE;
    playerPreviousY = playerY;
    playerPreviousX = playerX;

    if (encounter)
        return;

    if (inputs->up)
    {
        direction = UP;

        for (size_t l = 0; l < map->getLevels().size(); ++l)
        {
            auto& level = map->getLevels()[l];

            if (l != playerLevel)
                continue;

            auto& specialTileLayer = level->getSpecialTileLayer();
            auto& specialTile      = (*specialTileLayer.get())(playerX, playerY - 1);
            if (specialTile && *(specialTile.get()) != GRASS)
                continue;

            for (size_t h = 0; h < level->getTileLayers().size(); ++h)
            {
                auto& layer = level->getTileLayers()[h];

                if (layer->getType() != TileLayer::SOLID)
                    continue;

                if (playerY > 0)
                {
                    auto& tile = (*layer.get())(playerX, playerY - 1);
                    if (!tile)
                    {
                        playerY--;
                    }
                }
            }
        }
    }
    else if (inputs->down)
    {
        direction = DOWN;

        for (size_t l = 0; l < map->getLevels().size(); ++l)
        {
            auto& level = map->getLevels()[l];

            if (l != playerLevel)
                continue;

            auto& specialTileLayer = level->getSpecialTileLayer();
            auto& specialTile      = (*specialTileLayer.get())(playerX, playerY + 1);
            if (specialTile && *(specialTile.get()) != GRASS)
                continue;

            for (size_t h = 0; h < level->getTileLayers().size(); ++h)
            {
                auto& layer = level->getTileLayers()[h];

                if (layer->getType() != TileLayer::SOLID)
                    continue;

                if (playerY < int(map->getNRow() - 1))
                {
                    auto& tile = (*layer.get())(playerX, playerY + 1);
                    if (!tile)
                    {
                        playerY++;
                    }
                }
            }
        }
    }
    else if (inputs->left)
    {
        direction = LEFT;

        for (size_t l = 0; l < map->getLevels().size(); ++l)
        {
            auto& level = map->getLevels()[l];

            if (l != playerLevel)
                continue;

            auto& specialTileLayer = level->getSpecialTileLayer();
            auto& specialTile      = (*specialTileLayer.get())(playerX - 1, playerY);
            if (specialTile && *(specialTile.get()) != GRASS)
                continue;

            for (size_t h = 0; h < level->getTileLayers().size(); ++h)
            {
                auto& layer = level->getTileLayers()[h];

                if (layer->getType() != TileLayer::SOLID)
                    continue;

                if (playerX > 0)
                {
                    auto& tile = (*layer.get())(playerX - 1, playerY);
                    if (!tile)
                    {
                        playerX--;
                    }
                }
            }
        }
    }
    else if (inputs->right)
    {
        direction = RIGHT;

        for (size_t l = 0; l < map->getLevels().size(); ++l)
        {
            auto& level = map->getLevels()[l];

            if (l != playerLevel)
                continue;

            auto& specialTileLayer = level->getSpecialTileLayer();
            auto& specialTile      = (*specialTileLayer.get())(playerX + 1, playerY);
            if (specialTile && *(specialTile.get()) != GRASS)
                continue;

            for (size_t h = 0; h < level->getTileLayers().size(); ++h)
            {
                auto& layer = level->getTileLayers()[h];

                if (layer->getType() != TileLayer::SOLID)
                    continue;

                if (playerX < int(map->getNCol() - 1))
                {
                    auto& tile = (*layer.get())(playerX + 1, playerY);
                    if (!tile)
                    {
                        playerX++;
                    }
                }
            }
        }
    }
}

void MapScene::draw(Fps const* fps, RenderSizes rs)
{
    int dstTilePixelWidth  = TilePixelSize * rs.ww / rs.aw;
    int dstTilePixelHeight = TilePixelSize * rs.wh / rs.ah;

    SDL_Rect dstPlayerRect;
    dstPlayerRect.x = (rs.ww - dstTilePixelWidth) / 2;
    dstPlayerRect.y = (rs.wh - dstTilePixelHeight) / 2 - (PlayerPixelHeight - TilePixelSize) * rs.wh / rs.ah;
    dstPlayerRect.w = dstTilePixelWidth + 1;
    dstPlayerRect.h = PlayerPixelHeight * rs.wh / rs.ah + 1;

    int playerOffsetX =
        (rs.ww - dstTilePixelWidth) / 2
        - (playerPreviousX
           + (playerX - playerPreviousX) * (accumulatedTicks + fps->tickPercentage()) / ((speed + previousSpeed) / 2.0))
              * dstTilePixelWidth;
    int playerOffsetY =
        (rs.wh - dstTilePixelHeight) / 2
        - (playerPreviousY
           + (playerY - playerPreviousY) * (accumulatedTicks + fps->tickPercentage()) / ((speed + previousSpeed) / 2.0))
              * dstTilePixelHeight;

    for (size_t l = 0; l < map->getLevels().size(); ++l)
    {
        auto& level = map->getLevels()[l];

        for (size_t h = 0; h < level->getTileLayers().size(); ++h)
        {
            auto& layer = level->getTileLayers()[h];

            for (size_t i = 0; i < map->getNCol(); ++i)
            {
                for (size_t j = 0; j < map->getNRow(); ++j)
                {
                    auto& tile = (*layer.get())(i, j);
                    if (tile)
                    {
                        std::string path = tile->getSpritePath();
                        if (!sprites.count(path))
                        {
                            sprites[path].first  = IMG_Load(path.c_str());
                            sprites[path].second = SDL_CreateTextureFromSurface(renderer, sprites[path].first);
                        }

                        SDL_Rect srcRect;
                        srcRect.x = tile->getCol() * TilePixelSize;
                        srcRect.y = tile->getRow() * TilePixelSize;
                        srcRect.w = TilePixelSize;
                        srcRect.h = TilePixelSize;

                        SDL_Rect dstRect;
                        dstRect.x = i * dstTilePixelWidth + playerOffsetX;
                        dstRect.y = j * dstTilePixelHeight + playerOffsetY;
                        dstRect.w = dstTilePixelWidth + 1;
                        dstRect.h = dstTilePixelHeight + 1;

                        // draw only visible tiles
                        if (dstRect.x >= -dstTilePixelWidth && dstRect.x <= (rs.ww + dstTilePixelWidth)
                            && dstRect.y >= -dstTilePixelHeight && dstRect.y <= rs.wh + dstTilePixelHeight)
                        {
                            SDL_RenderCopy(renderer, sprites[path].second, &srcRect, &dstRect);
                        }
                    }
                }
            }

            if (playerLevel == l && layer->getType() == TileLayer::SOLID)
            {
                switch (direction)
                {
                case UP:
                    playerSpriteRow = 3;
                    break;
                case DOWN:
                    playerSpriteRow = 0;
                    break;
                case LEFT:
                    playerSpriteRow = 1;
                    break;
                case RIGHT:
                    playerSpriteRow = 2;
                    break;
                default:
                    break;
                }

                int imageCol = 0;
                if (direction != NONE)
                {
                    imageCol = std::floor((accumulatedTicks + fps->tickPercentage()) / speed * 4);
                }

                SDL_Rect srcRect;
                srcRect.x = imageCol * TilePixelSize;
                srcRect.y = playerSpriteRow * PlayerPixelHeight;
                srcRect.w = TilePixelSize;
                srcRect.h = PlayerPixelHeight;

                SDL_RenderCopy(renderer, playerTexture, &srcRect, &dstPlayerRect);
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
}

void MapScene::initPlayerPosition(int x, int y, Direction direction)
{
    playerX         = x;
    playerY         = y;
    playerPreviousX = x;
    playerPreviousY = y;
    this->direction = direction;
}

bool MapScene::manageEncounters()
{
    if (direction != NONE && !encounteredPkmn)
    {
        auto& level = map->getLevels()[playerLevel];

        auto& specialTileLayer = level->getSpecialTileLayer();
        auto& specialTile      = (*specialTileLayer.get())(playerX, playerY);
        if (specialTile && *(specialTile.get()) == GRASS)
        {
            // TODO all cases
            auto const& encounterMethods = map->getEncounterMethods();
            auto it = std::find_if(encounterMethods.begin(), encounterMethods.end(), [=](EncounterMethod const& e) {
                return e.getType() == EncounterMethod::Type::LAND;
            });
            if (it != encounterMethods.end())
            {
                auto const& encounterMethod = *it;
                size_t      pkmnEncounter   = randint(0, 99);

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
                        encounteredPkmn = std::make_shared<Pkmn>(pkmnDef, randint(e.getLevelMin(), e.getLevelMax()));
                    return true;
                }
            }
        }
    }

    return false;
}

bool MapScene::pushScene() const
{
    return encounteredPkmn != nullptr;
}

void MapScene::popReset()
{
    encounteredPkmn.reset();
}

std::unique_ptr<Scene> MapScene::nextScene()
{
    if (encounteredPkmn)
    {
        auto scene = std::make_unique<BattleScene>(renderer);
        return scene;
    }
    return nullptr;
}
