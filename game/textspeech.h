#ifndef TEXTSPEECH_H
#define TEXTSPEECH_H

#include "animations/textanimation.h"
#include "inputs.h"
#include <SDL_image.h>

class TextSpeech
{
public:
    static constexpr int TextBoxSize = 48;

    TextSpeech(SDL_Renderer* renderer);
    ~TextSpeech();

    void init();
    void update(const Inputs* inputs);
    void draw(const Fps* fps, RenderSizes rs);

    bool shouldClose() const;
    bool mayClose() const;
    void reset();

    void setTexts(const std::vector<std::string>& texts);

private:
    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;

    unsigned int currentAnimation = 0;

    std::vector<std::unique_ptr<TextAnimation>> animations;
};

#endif // TEXTSPEECH_H
