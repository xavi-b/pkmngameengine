#ifndef TEXTSPEECH_H
#define TEXTSPEECH_H

#include "animations/textanimation.h"
#include "inputs.h"
#include <SDL_image.h>

class TextSpeech
{
public:
    TextSpeech(SDL_Renderer* renderer, const std::vector<std::string>& texts);
    ~TextSpeech();

    void update(const Inputs* inputs);
    void draw(const Fps* fps, RenderSizes rs);

    bool isFinished() const;

private:
    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;

    unsigned int currentAnimation = 0;

    std::vector<std::unique_ptr<TextAnimation>> animations;
};

#endif // TEXTSPEECH_H
