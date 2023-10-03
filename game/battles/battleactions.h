#ifndef BATTLEACTIONS_H
#define BATTLEACTIONS_H

#include "fps.h"
#include "inputs.h"
#include "textspeech.h"

#include <SDL_image.h>
#include <string>
#include <vector>

class BattleActions
{
public:
    static constexpr int NElements    = 2;
    static constexpr int TextWidth    = 60;
    static constexpr int ArrowSize    = 16;
    static constexpr int TextPaddingX = 10;
    static constexpr int TextBoxWidth =
        TextSpeech::TextBoxBorderSize * 2 + (ArrowSize + TextWidth) * NElements + TextPaddingX * (NElements + 1);

    enum Type
    {
        MOVES,
        BAG,
        PKMNS,
        RUN,
        __SIZE
    };

    static constexpr size_t TypeCount = Type::__SIZE + Type::MOVES;
    static std::string      TypeToString(Type e);

    BattleActions(SDL_Renderer* renderer);
    ~BattleActions();

    void init();
    void update(Inputs const* inputs);
    void draw(Fps const* fps, RenderSizes rs);

    bool isFinished() const;
    void reset();

    int  selectedIndex() const;
    Type selectedAction() const;

private:
    SDL_Renderer* renderer;
    SDL_Surface*  bgSurface;
    SDL_Texture*  bgTexture;
    SDL_Surface*  selectionSurface;
    SDL_Texture*  selectionTexture;

    bool selected     = false;
    int  currentIndex = 0;
};

#endif // BATTLEACTIONS_H
