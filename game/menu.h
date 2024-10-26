#ifndef MENU_H
#define MENU_H

#include "fps.h"
#include "inputs.h"

#include <SDL_image.h>
#include <string>
#include <vector>

class Menu
{
public:
    static constexpr int ArrowSize    = 16;
    static constexpr int TextPaddingX = 10;

    enum Selection
    {
        PKMNS,
        BAG,
        SAVE,
        QUIT,
        __SIZE
    };

    static constexpr size_t SelectionCount = Selection::__SIZE;
    static std::string      SelectionToString(Selection e);

    Menu(SDL_Renderer* renderer);
    ~Menu();

    void init();
    void update(Inputs const* inputs);
    void draw(Fps const* fps, RenderSizes rs);

    bool isFinished() const;
    bool isSelected() const;
    void reset();

    Selection selection() const;

private:
    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;
    SDL_Surface*  selectionSurface;
    SDL_Texture*  selectionTexture;

    bool      finished         = false;
    bool      selected         = false;
    Selection currentSelection = SAVE;
};

#endif // MENU_H
