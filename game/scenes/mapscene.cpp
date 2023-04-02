#include "mapscene.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "game.h"

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
    accumulatedTicks = (accumulatedTicks + 1) % speed;
    if (accumulatedTicks != 0)
        return;

    // TODO block solid
    direction       = NONE;
    playerPreviousY = playerY;
    playerPreviousX = playerX;

    if (inputs->up)
    {
        direction = UP;

        if (playerY > 0)
            playerY--;
    }
    else if (inputs->down)
    {
        direction = DOWN;

        if (playerY < int(map->getNRow() - 1))
            playerY++;
    }
    else if (inputs->left)
    {
        direction = LEFT;

        if (playerX > 0)
            playerX--;
    }
    else if (inputs->right)
    {
        direction = RIGHT;

        if (playerX < int(map->getNCol() - 1))
            playerX++;
    }
}

void MapScene::draw(Fps const* fps, RenderSizes rs)
{
    int playerOffsetX =
        (playerPreviousX + (playerX - playerPreviousX) * (accumulatedTicks + fps->tickPercentage()) / speed) *
        TilePixelSize * rs.ww / rs.aw;
    int playerOffsetY =
        (playerPreviousY + (playerY - playerPreviousY) * (accumulatedTicks + fps->tickPercentage()) / speed) *
        TilePixelSize * rs.wh / rs.ah;

    for (size_t l = 0; l < map->getLevels().size(); ++l)
    {
        auto& level = map->getLevels()[l];

        for (size_t h = 0; h < level->getTileLayers().size(); ++h)
        {
            auto& layer = level->getTileLayers()[h];

            // TODO draw only visible tiles
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
                        dstRect.x = i * TilePixelSize * rs.ww / rs.aw - playerOffsetX;
                        dstRect.y = j * TilePixelSize * rs.wh / rs.ah - playerOffsetY;
                        dstRect.w = TilePixelSize * rs.ww / rs.aw + 1;
                        dstRect.h = TilePixelSize * rs.wh / rs.ah + 1;

                        SDL_RenderCopy(renderer, sprites[path].second, &srcRect, &dstRect);
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

                SDL_Rect dstRect;
                int      dstTilePixelWidth  = TilePixelSize * rs.ww / rs.aw;
                int      dstTilePixelHeight = TilePixelSize * rs.wh / rs.ah;

                dstRect.x = (rs.ww - dstTilePixelWidth) / 2;
                dstRect.y = (rs.wh - dstTilePixelHeight) / 2 - (PlayerPixelHeight - TilePixelSize) * rs.wh / rs.ah;
                dstRect.w = dstTilePixelWidth + 1;
                dstRect.h = PlayerPixelHeight * rs.wh / rs.ah + 1;

                SDL_RenderCopy(renderer, playerTexture, &srcRect, &dstRect);
            }
        }
    }
}
