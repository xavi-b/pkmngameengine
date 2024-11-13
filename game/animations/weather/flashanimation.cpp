#include "flashanimation.h"

#include <iostream>

FlashAnimation::FlashAnimation(SDL_Renderer* renderer) : Animation(renderer)
{
    fileSurface = IMG_Load("resources/Graphics/Transitions/flash.png");
    fileTexture = SDL_CreateTextureFromSurface(renderer, fileSurface);
}

FlashAnimation::~FlashAnimation()
{
    if (fileTexture)
        SDL_DestroyTexture(fileTexture);
    if (fileSurface)
        SDL_FreeSurface(fileSurface);
}

bool FlashAnimation::isFinished() const
{
    if (!fileTexture || !fileSurface)
        return true;

    if (accumulatedTicks > ticksDuration)
        return true;

    return Animation::isFinished();
}

void FlashAnimation::draw(Fps const* fps, RenderSizes /*rs*/)
{
    if (!fileTexture || !fileSurface)
        return;

    float durationPercentage = (accumulatedTicks + 1 + (isStarted() ? fps->tickPercentage() : 0)) / ticksDuration;
    if (durationPercentage > 1.0)
        durationPercentage = 1.0;

    unsigned char threshold = (1.0 - durationPercentage) * 255;

    SDL_Surface* formattedImage = SDL_ConvertSurfaceFormat(fileSurface, SDL_PIXELFORMAT_ARGB8888, 0);

    SDL_Texture* formattedTexure =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, fileSurface->w, fileSurface->h);
    SDL_SetTextureBlendMode(formattedTexure, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, formattedTexure);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    int width  = formattedImage->w;
    int height = formattedImage->h;

    SDL_LockSurface(formattedImage);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    Uint32* pixels = (Uint32*)formattedImage->pixels;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            Uint32 colors = pixels[width * y + x];
            Uint8  color  = (colors & 0x000000FF);
            if (color - 1 < threshold)
            {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }

    SDL_UnlockSurface(formattedImage);

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, formattedTexure, NULL, NULL);

    SDL_DestroyTexture(formattedTexure);
    SDL_FreeSurface(formattedImage);
}
