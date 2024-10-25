#ifndef MOVESELECTION_H
#define MOVESELECTION_H

#include "fps.h"
#include "inputs.h"
#include "pkmn.h"
#include "textspeech.h"

#include <SDL_image.h>
#include <string>
#include <vector>

class MoveSelection
{
public:
    static constexpr int NElements    = 2;
    static constexpr int TextWidth    = 60;
    static constexpr int ArrowSize    = 16;
    static constexpr int TextPaddingX = 10;

    static constexpr size_t MaxMoveCount = 4;

    MoveSelection(SDL_Renderer* renderer);
    ~MoveSelection();

    void init();
    void update(Inputs const* inputs);
    void draw(Fps const* fps, RenderSizes rs);

    bool isSelected() const;
    void reset();

    int selectedIndex() const;

    bool shouldQuit() const;

    void setPkmn(Pkmn::PkmnPtr const& pkmn);

private:
    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;
    SDL_Surface*  selectionSurface;
    SDL_Texture*  selectionTexture;

    bool selected     = false;
    int  currentIndex = 0;
    bool quit         = false;

    Pkmn::PkmnPtr pkmn;
};

#endif // MOVESELECTION_H
