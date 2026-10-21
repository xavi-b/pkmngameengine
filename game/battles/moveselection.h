#ifndef MOVESELECTION_H
#define MOVESELECTION_H

#include "fps.h"
#include "scenes/battle/battlebattler.h"
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
    static constexpr int TextPaddingX = 0;

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
    void setBattlers(BattleBattler const* userBattler, Pkmn::PkmnPtr const& foe, BattleBattler const* foeBattler);
    void setGravity(bool gravity);

    bool moveUsable(size_t index) const;
    bool anyMoveUsable() const;

    size_t inputId() const
    {
        return inputToken;
    }

private:
    size_t inputToken = Inputs::newToken();

    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;
    SDL_Surface*  selectionSurface;
    SDL_Texture*  selectionTexture;

    bool selected     = false;
    int  currentIndex = 0;
    bool quit         = false;

    Pkmn::PkmnPtr          pkmn;
    BattleBattler const*   userBattler = nullptr;
    Pkmn::PkmnPtr          foe;
    BattleBattler const*   foeBattler  = nullptr;
    bool                   sceneGravity = false;
};

#endif // MOVESELECTION_H
