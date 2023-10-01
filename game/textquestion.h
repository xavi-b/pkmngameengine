#ifndef TEXTQUESTION_H
#define TEXTQUESTION_H

#include "fps.h"
#include "inputs.h"

#include <SDL_image.h>
#include <string>
#include <vector>

class TextQuestion
{
public:
    static constexpr int ArrowSize    = 16;
    static constexpr int TextPaddingX = 10;

    enum Position
    {
        RIGHT,
        LEFT
    };

    TextQuestion(SDL_Renderer* renderer);
    ~TextQuestion();

    void init();
    void update(Inputs const* inputs);
    void draw(Fps const* fps, RenderSizes rs, Position pos = RIGHT);

    bool isFinished() const;
    void reset();

    void setTexts(std::vector<std::string> const& texts);
    int  selectedIndex() const;

private:
    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;
    SDL_Surface*  selectionSurface;
    SDL_Texture*  selectionTexture;

    bool selected     = false;
    int  currentIndex = 0;

    std::vector<std::string> texts;
};

#endif // TEXTQUESTION_H
