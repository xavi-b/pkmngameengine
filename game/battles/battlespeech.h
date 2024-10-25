#ifndef BATTLESPEECH_H
#define BATTLESPEECH_H

#include "animations/textanimation.h"
#include "inputs.h"
#include "textspeech.h"

#include <SDL_image.h>

class BattleSpeech
{
public:
    static constexpr int TextBoxSize = 48;

    BattleSpeech(SDL_Renderer* renderer);
    ~BattleSpeech();

    void init();
    void update(Inputs const* inputs);
    void draw(Fps const* fps, RenderSizes rs);

    bool mayClose() const;
    void reset();

    void setTexts(std::vector<std::string> const& texts);

private:
    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;

    unsigned int currentAnimation = 0;

    std::vector<std::unique_ptr<TextAnimation>> animations;
};

#endif // BATTLESPEECH_H
