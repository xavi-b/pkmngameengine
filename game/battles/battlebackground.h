#ifndef BATTLEBACKGROUND_H
#define BATTLEBACKGROUND_H

#include "fps.h"
#include "inputs.h"

#include <SDL_image.h>
#include <string>
#include <vector>

class BattleBackground
{
public:
    BattleBackground(SDL_Renderer* renderer);
    ~BattleBackground();

    void update(Inputs const* inputs);
    void draw(Fps const* fps, RenderSizes rs);

private:
    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;
    SDL_Surface*  base1Surface;
    SDL_Texture*  base1Texture;
};

#endif // BATTLEBACKGROUND_H
