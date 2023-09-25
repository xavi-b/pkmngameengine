#ifndef BATTLEACTIONS_H
#define BATTLEACTIONS_H

#include "fps.h"
#include "inputs.h"

#include <SDL_image.h>
#include <string>
#include <vector>

class BattleActions
{
public:
    enum Type
    {
        FIGHT,
        BAG,
        PKNMS,
        RUN,
        __SIZE
    };

    static constexpr size_t TypeCount = Type::__SIZE + Type::FIGHT;
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
