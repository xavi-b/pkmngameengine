#include "mapscene.h"

#include <fstream>
#include <sstream>

MapScene::MapScene(SDL_Renderer* renderer, std::string const& mapPath) : Scene(renderer)
{
    std::ifstream     t(mapPath);
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto mapPtr = js::value_to<std::unique_ptr<Map>>(js::parse(buffer.str()));
    map.swap(mapPtr);
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

void MapScene::draw(Fps const* /*fps*/, RenderSizes rs)
{
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
                        srcRect.x = tile->getRow() * TilePixelSize;
                        srcRect.y = tile->getCol() * TilePixelSize;
                        srcRect.w = TilePixelSize;
                        srcRect.h = TilePixelSize;

                        SDL_Rect dstRect;
                        dstRect.x = i * TilePixelSize * rs.ww / rs.aw;
                        dstRect.y = j * TilePixelSize * rs.wh / rs.ah;
                        dstRect.w = TilePixelSize * rs.ww / rs.aw + 1;
                        dstRect.h = TilePixelSize * rs.wh / rs.ah + 1;

                        SDL_RenderCopy(renderer, sprites[path].second, &srcRect, &dstRect);
                    }
                }
            }
        }
    }
}
