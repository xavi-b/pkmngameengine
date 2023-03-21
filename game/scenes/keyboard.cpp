#include "keyboard.h"

#include "settings.h"
#include "game.h"
#include <iostream>

Keyboard::Keyboard(SDL_Renderer* renderer, std::string& playerName) : Scene(renderer), playerName(playerName)
{
    bgSurface         = IMG_Load("resources/Graphics/Pictures/Naming/bg.png");
    bgTexture         = SDL_CreateTextureFromSurface(renderer, bgSurface);
    keyboardBgSurface = IMG_Load("resources/Graphics/Pictures/Naming/overlay_tab_1.png");
    keyboardBgTexture = SDL_CreateTextureFromSurface(renderer, keyboardBgSurface);
}

Keyboard::~Keyboard()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(keyboardBgTexture);
    SDL_FreeSurface(keyboardBgSurface);
}

void Keyboard::init()
{
}

void Keyboard::update(const Inputs* inputs)
{
    if (inputs->A)
    {
        playerName = "Red";
        leave      = true;
    }
}

void Keyboard::draw(const Fps* /*fps*/, RenderSizes rs)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    const int topMargin    = 96;
    const int dstTopMargin = topMargin * rs.ww / rs.aw;

    SDL_Rect dstRect;
    dstRect.x = (rs.ww - keyboardBgSurface->w * rs.ww / rs.aw) / 2;
    dstRect.y = dstTopMargin + (rs.wh - dstTopMargin - keyboardBgSurface->h * rs.wh / rs.ah) / 2;
    dstRect.w = keyboardBgSurface->w * rs.ww / rs.aw;
    dstRect.h = keyboardBgSurface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, keyboardBgTexture, NULL, &dstRect);

    int boxWidth   = 32;
    int boxHeight  = 38;
    int nbRowBoxes = 11;
    int startX     = 60;
    int startY     = 52;

    for (char c = 'a'; c <= 'z'; ++c)
    {
        int pos      = c - 'a';
        int col      = pos % nbRowBoxes;
        int row      = pos / nbRowBoxes;
        int fontSize = 24;

        SDL_Rect textRect;
        textRect.x = dstRect.x + (startX + col * boxWidth) * rs.ww / rs.aw;
        textRect.y = dstRect.y + (startY + row * boxHeight + (boxHeight - fontSize - 6) / 2) * rs.wh / rs.ah;
        textRect.w = boxWidth * rs.ww / rs.aw;
        RenderUtils::drawWhiteTextCentered(renderer, rs, std::string() + c, fontSize, textRect);
    }
}

bool Keyboard::popScene() const
{
    return leave;
}

std::unique_ptr<Scene> Keyboard::nextScene()
{
    return nullptr;
}

std::string Keyboard::name()
{
    return "Keyboard";
}
