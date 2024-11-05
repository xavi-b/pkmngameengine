#include "rainanimation.h"

#include "utils.h"

#include <SDL_image.h>

RainAnimation::RainAnimation(SDL_Renderer* renderer) : WeatherAnimation(renderer)
{
    for (size_t i = 1; i <= 3; ++i)
    {
        SDL_Surface* surface = IMG_Load(("resources/Graphics/Weather/rain_" + std::to_string(i) + ".png").c_str());
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        rains.emplace(surface, texture);
    }
}

RainAnimation::~RainAnimation()
{
    for (auto it = rains.begin(); it != rains.end(); ++it)
    {
        SDL_DestroyTexture(it->second);
        SDL_FreeSurface(it->first);
    }
}

std::string RainAnimation::getStartingText() const
{
    return lc::translate("It started to rain !");
}

std::string RainAnimation::getContinuingText() const
{
    return lc::translate("Rain continues to fall.");
}

void RainAnimation::start()
{
    WeatherAnimation::start();

    for (size_t i = 0; i < NumberOfRains; ++i)
    {
        size_t r      = Utils::randuint(0, rains.size() - 1);
        auto   rainIt = rains.begin();
        std::advance(rainIt, r);

        selected[i] = {true, 0, 0, 0, rainIt->first, rainIt->second};
    }
}

void RainAnimation::incrementTicks()
{
    WeatherAnimation::incrementTicks();

    if (int(accumulatedTicks) % spawnTick == 0)
    {
        for (size_t i = 0; i < NumberOfRains; ++i)
        {
            size_t j = Utils::randuint(0, NumberOfRains - 1);

            size_t r      = Utils::randuint(0, rains.size() - 1);
            auto   rainIt = rains.begin();
            std::advance(rainIt, r);

            selected[j] = {true, 0, 0, 0, rainIt->first, rainIt->second};
        }
    }

    for (size_t i = 0; i < selected.size(); ++i)
    {
        auto& [isNew, baseX, baseY, ticks, rainSurface, rainTexture] = selected[i];

        if (!rainSurface)
            continue;

        if (isNew)
            continue;

        ticks++;
    }
}

void RainAnimation::draw(Fps const* fps, RenderSizes rs)
{
    for (size_t i = 0; i < selected.size(); ++i)
    {
        auto& [isNew, baseX, baseY, ticks, rainSurface, rainTexture] = selected[i];

        if (!rainSurface)
            continue;

        if (isNew)
        {
            baseX = Utils::randint(-rainSurface->w * 2, rs.aw + rainSurface->w);
            baseY = Utils::randint(-rainSurface->h * 2, rs.ah + rainSurface->h);
        }

        isNew = false;

        SDL_Rect dstRect;
        dstRect.w = rainSurface->w * rs.ww / rs.aw;
        dstRect.h = rainSurface->h * rs.wh / rs.ah;
        dstRect.x = (baseX - (ticks + fps->tickPercentage()) * rainSurface->w / speed) * rs.ww / rs.aw;
        dstRect.y = (baseY + (ticks + fps->tickPercentage()) * rainSurface->h / speed) * rs.wh / rs.ah;
        SDL_RenderCopy(renderer, rainTexture, NULL, &dstRect);
    }
}
