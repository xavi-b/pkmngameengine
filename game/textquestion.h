#ifndef TEXTQUESTION_H
#define TEXTQUESTION_H

#include "inputs.h"
#include <vector>
#include <string>
#include <SDL_image.h>
#include "fps.h"

class TextQuestion
{
public:
    enum Position
    {
        Right,
        Left
    };

    TextQuestion(SDL_Renderer* renderer);
    ~TextQuestion();

    void init();
    void update(Inputs const* inputs);
    void draw(Fps const* fps, RenderSizes rs, Position pos = Right);

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
