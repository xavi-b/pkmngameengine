#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "scene.h"
#include <string>
#include <SDL_image.h>

class Keyboard : public Scene
{
public:
    Keyboard(SDL_Renderer* renderer, std::string& textOut, size_t maxSize = 8);
    virtual ~Keyboard();

    virtual void init() override;
    virtual void update(const Inputs* inputs) override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    static constexpr int NbRowBoxes = 11;
    static constexpr int MaxPos     = 'z' - 'a';

    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;
    SDL_Surface* keyboardBgSurface;
    SDL_Texture* keyboardBgTexture;
    SDL_Surface* cursorSurface;
    SDL_Texture* cursorTexture;

    bool leave      = false;
    int  currentPos = 0;

    std::string& textOut;
    size_t       maxSize = 8;
};

#endif // KEYBOARD_H
